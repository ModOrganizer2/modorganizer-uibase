/*
Mod Organizer shared UI functionality

Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "utility.h"
#include "report.h"
#include "log.h"
#include <memory>
#include <sstream>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <QDir>
#include <QBuffer>
#include <QDesktopWidget>
#include <QApplication>
#include <QTextCodec>
#include <QtDebug>
#include <QUuid>
#include <QtWinExtras/QtWin>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


#define FO_RECYCLE 0x1003


namespace MOBase {


MyException::MyException(const QString &text)
  : std::exception(), m_Message(text.toUtf8())
{
}


bool removeDir(const QString &dirName)
{
  QDir dir(dirName);

  if (dir.exists()) {
    Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
      if (info.isDir()) {
        if (!removeDir(info.absoluteFilePath())) {
          return false;
        }
      } else {
        ::SetFileAttributesW(ToWString(info.absoluteFilePath()).c_str(), FILE_ATTRIBUTE_NORMAL);
        QFile file(info.absoluteFilePath());
        if (!file.remove()) {
          reportError(QObject::tr("removal of \"%1\" failed: %2").arg(info.absoluteFilePath()).arg(file.errorString()));
          return false;
        }
      }
    }

    if (!dir.rmdir(dirName)) {
      reportError(QObject::tr("removal of \"%1\" failed").arg(dir.absolutePath()));
      return false;
    }
  } else {
    reportError(QObject::tr("\"%1\" doesn't exist (remove)").arg(dirName));
    return false;
  }

  return true;
}


bool copyDir(const QString &sourceName, const QString &destinationName, bool merge)
{
  QDir sourceDir(sourceName);
  if (!sourceDir.exists()) {
    return false;
  }
  QDir destDir(destinationName);
  if (!destDir.exists()) {
    destDir.mkdir(destinationName);
  } else if (!merge) {
    return false;
  }

  QStringList files = sourceDir.entryList(QDir::Files);
  foreach (QString fileName, files) {
    QString srcName = sourceName + "/" + fileName;
    QString destName = destinationName + "/" + fileName;
    QFile::copy(srcName, destName);
  }

  files.clear();
  // we leave out symlinks because that could cause an endless recursion
  QStringList subDirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  foreach (QString subDir, subDirs) {
    QString srcName = sourceName + "/" + subDir;
    QString destName = destinationName + "/" + subDir;
    copyDir(srcName, destName, merge);
  }
  return true;
}


static DWORD TranslateError(int error)
{
  switch (error) {
    case 0x71:    return ERROR_INVALID_PARAMETER; // same file
    case 0x72:    return ERROR_INVALID_PARAMETER; // many source, one destination. shouldn't happen due to how parameters are transformed
    case 0x73:    return ERROR_NOT_SAME_DEVICE;
    case 0x74:    return ERROR_INVALID_PARAMETER;
    case 0x75:    return ERROR_CANCELLED;
    case 0x76:    return ERROR_BAD_PATHNAME;
    case 0x78:    return ERROR_ACCESS_DENIED;
    case 0x79:    return ERROR_BUFFER_OVERFLOW; // path exceeds max_path
    case 0x7A:    return ERROR_INVALID_PARAMETER;
    case 0x7C:    return ERROR_BAD_PATHNAME;
    case 0x7D:    return ERROR_INVALID_PARAMETER;
    case 0x7E:    return ERROR_ALREADY_EXISTS;
    case 0x80:    return ERROR_ALREADY_EXISTS;
    case 0x81:    return ERROR_BUFFER_OVERFLOW;
    case 0x82:    return ERROR_WRITE_PROTECT;
    case 0x83:    return ERROR_WRITE_PROTECT;
    case 0x84:    return ERROR_WRITE_PROTECT;
    case 0x85:    return ERROR_DISK_FULL;
    case 0x86:    return ERROR_WRITE_PROTECT;
    case 0x87:    return ERROR_WRITE_PROTECT;
    case 0x88:    return ERROR_WRITE_PROTECT;
    case 0xB7:    return ERROR_BUFFER_OVERFLOW;
    case 0x402:   return ERROR_PATH_NOT_FOUND;
    case 0x10000: return ERROR_GEN_FAILURE;
    default: return static_cast<DWORD>(error);
  }
}


static bool shellOp(const QStringList &sourceNames, const QStringList &destinationNames, QWidget *dialog, UINT operation, bool yesToAll)
{
  std::vector<wchar_t> fromBuffer;
  std::vector<wchar_t> toBuffer;

  foreach (const QString &from, sourceNames) {
    // SHFileOperation has to be used with absolute maths, err paths ("It cannot be overstated" they say)
    std::wstring tempFrom = ToWString(QDir::toNativeSeparators(QFileInfo(from).absoluteFilePath()));
    fromBuffer.insert(fromBuffer.end(), tempFrom.begin(), tempFrom.end());
    fromBuffer.push_back(L'\0');
  }

  bool recycle = operation == FO_RECYCLE;

  if (recycle) {
    operation = FO_DELETE;
  }

  if ((destinationNames.count() == sourceNames.count()) ||
      (destinationNames.count() == 1)) {
    foreach (const QString &to, destinationNames) {
      std::wstring tempTo = ToWString(QDir::toNativeSeparators(QFileInfo(to).absoluteFilePath()));
      toBuffer.insert(toBuffer.end(), tempTo.begin(), tempTo.end());
      toBuffer.push_back(L'\0');
    }
  } else if ((operation == FO_DELETE) && (destinationNames.count() == 0)) {
    // pTo is not used but as I understand the documentation it should still be double-null terminated
    toBuffer.push_back(L'\0');
  } else {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    return false;
  }

  // both buffers have to be double-null terminated
  fromBuffer.push_back(L'\0');
  toBuffer.push_back(L'\0');

  SHFILEOPSTRUCTW op;
  memset(&op, 0, sizeof(SHFILEOPSTRUCTW));
  if (dialog != nullptr) {
    op.hwnd = (HWND)dialog->winId();
  } else {
    op.hwnd = nullptr;
  }
  op.wFunc = operation;
  op.pFrom = &fromBuffer[0];
  op.pTo = &toBuffer[0];

  if ((operation == FO_DELETE) || yesToAll) {
    op.fFlags = FOF_NOCONFIRMATION;
    if (recycle) {
      op.fFlags |= FOF_ALLOWUNDO;
    }
  } else {
    op.fFlags = FOF_NOCOPYSECURITYATTRIBS |  // always use security of target directory
                FOF_SILENT |                 // don't show a progress bar
                FOF_NOCONFIRMMKDIR;          // silently create directories

    if (destinationNames.count() == sourceNames.count()) {
      op.fFlags |= FOF_MULTIDESTFILES;
    }
  }

  int res = ::SHFileOperationW(&op);
  if (res == 0) {
    return true;
  } else {
    ::SetLastError(TranslateError(res));
    return false;
  }
}

bool shellCopy(const QStringList &sourceNames, const QStringList &destinationNames, QWidget *dialog)
{
  return shellOp(sourceNames, destinationNames, dialog, FO_COPY, false);
}

bool shellCopy(const QString &sourceNames, const QString &destinationNames, bool yesToAll, QWidget *dialog)
{
  return shellOp(QStringList() << sourceNames, QStringList() << destinationNames, dialog, FO_COPY, yesToAll);
}

bool shellMove(const QStringList &sourceNames, const QStringList &destinationNames, QWidget *dialog)
{
  return shellOp(sourceNames, destinationNames, dialog, FO_MOVE, false);
}

bool shellMove(const QString &sourceNames, const QString &destinationNames, bool yesToAll, QWidget *dialog)
{
  return shellOp(QStringList() << sourceNames, QStringList() << destinationNames, dialog, FO_MOVE, yesToAll);
}

bool shellRename(const QString &oldName, const QString &newName, bool yesToAll, QWidget *dialog)
{
  return shellOp(QStringList(oldName), QStringList(newName), dialog, FO_RENAME, yesToAll);
}

bool shellDelete(const QStringList &fileNames, bool recycle, QWidget *dialog)
{
  const UINT op = static_cast<UINT>(recycle ? FO_RECYCLE : FO_DELETE);
  return shellOp(fileNames, QStringList(), dialog, op, false);
}


namespace shell
{

Result::Result(bool success, DWORD error, QString message, HANDLE process) :
  m_success(success), m_error(error), m_message(std::move(message)),
  m_process(process)
{
  if (m_message.isEmpty()) {
    m_message = QString::fromStdWString(formatSystemMessage(m_error));
  }
}

Result Result::makeFailure(DWORD error, QString message)
{
  return Result(false, error, std::move(message), INVALID_HANDLE_VALUE);
}

Result Result::makeSuccess(HANDLE process)
{
  return Result(true, ERROR_SUCCESS, {}, process);
}

bool Result::success() const
{
  return m_success;
}

Result::operator bool() const
{
  return m_success;
}

DWORD Result::error()
{
  return m_error;
}

const QString& Result::message() const
{
  return m_message;
}

HANDLE Result::processHandle() const
{
  return m_process.get();
}

HANDLE Result::stealProcessHandle()
{
  const auto h = m_process.release();
  m_process.reset(INVALID_HANDLE_VALUE);
  return h;
}

QString Result::toString() const
{
  if (m_message.isEmpty()) {
    return QObject::tr("Error %1").arg(m_error);
  } else {
    return m_message;
  }
}


QString formatError(int i)
{
  switch (i) {
    case 0:
      return "The operating system is out of memory or resources";

    case ERROR_FILE_NOT_FOUND:
      return "The specified file was not found";

    case ERROR_PATH_NOT_FOUND:
      return "The specified path was not found";

    case ERROR_BAD_FORMAT:
      return "The .exe file is invalid (non-Win32 .exe or error in .exe image)";

    case SE_ERR_ACCESSDENIED:
      return "The operating system denied access to the specified file";

    case SE_ERR_ASSOCINCOMPLETE:
      return "The file name association is incomplete or invalid";

    case SE_ERR_DDEBUSY:
      return "The DDE transaction could not be completed because other DDE "
        "transactions were being processed";

    case SE_ERR_DDEFAIL:
      return "The DDE transaction failed";

    case SE_ERR_DDETIMEOUT:
      return "The DDE transaction could not be completed because the request "
        "timed out";

    case SE_ERR_DLLNOTFOUND:
      return "The specified DLL was not found";

    case SE_ERR_NOASSOC:
      return "There is no application associated with the given file name "
        "extension";

    case SE_ERR_OOM:
      return "There was not enough memory to complete the operation";

    case SE_ERR_SHARE:
      return "A sharing violation occurred";

    default:
      return QString("Unknown error %1").arg(i);
  }
}

void LogShellFailure(
  const wchar_t* operation, const wchar_t* file, const wchar_t* params,
  DWORD error)
{
  QString s;

  if (operation) {
    s += " " + QString::fromWCharArray(operation);
  }

  if (file) {
    s += " " + QString::fromWCharArray(file);
  }

  if (params) {
    s += " " + QString::fromWCharArray(params);
  }

  log::error("failed to invoke '{}': {}", s, formatSystemMessage(error));
}

Result ShellExecuteWrapper(
  const wchar_t* operation, const wchar_t* file, const wchar_t* params)
{
  SHELLEXECUTEINFOW info = {};

  info.cbSize = sizeof(info);
  info.fMask = SEE_MASK_FLAG_NO_UI | SEE_MASK_NOCLOSEPROCESS;
  info.lpVerb = operation;
  info.lpFile = file;
  info.lpParameters = params;
  info.nShow = SW_SHOWNORMAL;

  const auto r = ::ShellExecuteExW(&info);

  if (!r)
  {
    const auto e = ::GetLastError();
    LogShellFailure(operation, file, params, e);

    return Result::makeFailure(
      e, QString::fromStdWString(formatSystemMessage(e)));
  }

  const HANDLE process = info.hProcess ? info.hProcess : INVALID_HANDLE_VALUE;
  return Result::makeSuccess(process);
}

Result ExploreDirectory(const QFileInfo& info)
{
  const auto path = QDir::toNativeSeparators(info.absoluteFilePath());
  const auto ws_path = path.toStdWString();

  return ShellExecuteWrapper(L"explore", ws_path.c_str(), nullptr);
}

Result ExploreFileInDirectory(const QFileInfo& info)
{
  const auto path = QDir::toNativeSeparators(info.absoluteFilePath());
  const auto params = "/select,\"" + path + "\"";
  const auto ws_params = params.toStdWString();

  return ShellExecuteWrapper(nullptr, L"explorer", ws_params.c_str());
}


Result Explore(const QFileInfo& info)
{
  if (info.isFile()) {
    return ExploreFileInDirectory(info);
  } else if (info.isDir()) {
    return ExploreDirectory(info);
  } else {
    // try the parent directory
    const auto parent = info.dir();

    if (parent.exists()) {
      return ExploreDirectory(parent.absolutePath());
    } else {
      return Result::makeFailure(ERROR_FILE_NOT_FOUND);
    }
  }
}

Result Explore(const QString& path)
{
  return Explore(QFileInfo(path));
}

Result Explore(const QDir& dir)
{
  return Explore(QFileInfo(dir.absolutePath()));
}

Result Open(const QString& path)
{
  const auto ws_path = path.toStdWString();
  return ShellExecuteWrapper(L"open", ws_path.c_str(), nullptr);
}

Result Open(const QUrl& url)
{
  const auto ws_url = url.toString().toStdWString();
  return ShellExecuteWrapper(L"open", ws_url.c_str(), nullptr);
}

Result Execute(const QString& program, const QString& params)
{
  const auto program_ws = program.toStdWString();
  const auto params_ws = params.toStdWString();

  return ShellExecuteWrapper(L"open", program_ws.c_str(), params_ws.c_str());
}

std::wstring toUNC(const QFileInfo& path)
{
  auto wpath = QDir::toNativeSeparators(path.absoluteFilePath()).toStdWString();
  if (!wpath.starts_with(L"\\\\?\\")) {
    wpath = L"\\\\?\\" + wpath;
  }

  return wpath;
}

Result Delete(const QFileInfo& path)
{
  const auto wpath = toUNC(path);

  if (!::DeleteFileW(wpath.c_str())) {
    const auto e = ::GetLastError();
    return Result::makeFailure(e);
  }

  return Result::makeSuccess();
}

Result Rename(const QFileInfo& src, const QFileInfo& dest)
{
  const auto wsrc = toUNC(src);
  const auto wdest = toUNC(dest);

  if (!::MoveFileEx(wsrc.c_str(), wdest.c_str(), MOVEFILE_COPY_ALLOWED)) {
    const auto e = ::GetLastError();
    return Result::makeFailure(e);
  }

  return Result::makeSuccess();
}

} // namespace shell


bool moveFileRecursive(const QString &source, const QString &baseDir, const QString &destination)
{
  QStringList pathComponents = destination.split("/");
  QString path = baseDir;
  for (QStringList::Iterator iter = pathComponents.begin(); iter != pathComponents.end() - 1; ++iter) {
    path.append("/").append(*iter);
    if (!QDir(path).exists() && !QDir().mkdir(path)) {
      reportError(QObject::tr("failed to create directory \"%1\"").arg(path));
      return false;
    }
  }

  QString destinationAbsolute = baseDir.mid(0).append("/").append(destination);
  if (!QFile::rename(source, destinationAbsolute)) {
    // move failed, try copy & delete
    if (!QFile::copy(source, destinationAbsolute)) {
      reportError(QObject::tr("failed to copy \"%1\" to \"%2\"").arg(source).arg(destinationAbsolute));
      return false;
    } else {
      QFile::remove(source);
    }
  }
  return true;
}

bool copyFileRecursive(const QString &source, const QString &baseDir, const QString &destination)
{
  QStringList pathComponents = destination.split("/");
  QString path = baseDir;
  for (QStringList::Iterator iter = pathComponents.begin(); iter != pathComponents.end() - 1; ++iter) {
    path.append("/").append(*iter);
    if (!QDir(path).exists() && !QDir().mkdir(path)) {
      reportError(QObject::tr("failed to create directory \"%1\"").arg(path));
      return false;
    }
  }

  QString destinationAbsolute = baseDir.mid(0).append("/").append(destination);
  if (!QFile::copy(source, destinationAbsolute)) {
    reportError(QObject::tr("failed to copy \"%1\" to \"%2\"").arg(source).arg(destinationAbsolute));
    return false;
  }
  return true;
}


std::wstring ToWString(const QString &source)
{
  //FIXME
  //why not source.toStdWString() ?
  wchar_t *buffer = new wchar_t[static_cast<std::size_t>(source.count()) + 1];
  source.toWCharArray(buffer);
  buffer[source.count()] = L'\0';
  std::wstring result(buffer);
  delete [] buffer;

  return result;
}

std::string ToString(const QString &source, bool utf8)
{
  QByteArray array8bit;
  if (utf8) {
    array8bit = source.toUtf8();
  } else {
    array8bit = source.toLocal8Bit();
  }
  return std::string(array8bit.constData());
}

QString ToQString(const std::string &source)
{
  //return QString::fromUtf8(source.c_str());
  return QString::fromStdString(source);
}

QString ToQString(const std::wstring &source)
{
  //return QString::fromWCharArray(source.c_str());
  return QString::fromStdWString(source);
}

QString ToString(const SYSTEMTIME &time)
{
  char dateBuffer[100];
  char timeBuffer[100];
  int size = 100;
  GetDateFormatA(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &time, nullptr, dateBuffer, size);
  GetTimeFormatA(LOCALE_USER_DEFAULT, LOCALE_USE_CP_ACP, &time, nullptr, timeBuffer, size);
  return QString::fromLocal8Bit(dateBuffer) + " " + QString::fromLocal8Bit(timeBuffer);
}

bool fixDirectoryName(QString &name)
{
  QString temp = name.simplified();
  while (temp.endsWith('.')) temp.chop(1);

  temp.replace(QRegExp("[<>:\"/\\|?*]"), "");
  static QString invalidNames[] = { "CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                                    "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9" };
  for (unsigned int i = 0; i < sizeof(invalidNames) / sizeof(QString); ++i) {
    if (temp == invalidNames[i]) {
      temp = "";
      break;
    }
  }

  temp = temp.simplified();

  if (temp.length() >= 1) {
    name = temp;
    return true;
  } else {
    return false;
  }
}


struct CoTaskMemFreer
{
  void operator()(void* p)
  {
    ::CoTaskMemFree(p);
  }
};

template <class T>
using COMMemPtr = std::unique_ptr<T, CoTaskMemFreer>;


QDir getKnownFolder(KNOWNFOLDERID id, const QString& what)
{
  COMMemPtr<wchar_t> path;

  {
    wchar_t* rawPath = nullptr;
    HRESULT res = SHGetKnownFolderPath(id, 0, nullptr, &rawPath);

    if (FAILED(res)) {
      log::error(
        "failed to get known folder '{}', {}",
        what.isEmpty() ? QUuid(id).toString() : what,
        formatSystemMessage(res));

      throw std::runtime_error("couldn't get known folder path");
    }

    path.reset(rawPath);
  }

  return QString::fromWCharArray(path.get());
}

QString getDesktopDirectory()
{
  return getKnownFolder(FOLDERID_Desktop, "desktop").absolutePath();
}

QString getStartMenuDirectory()
{
  return getKnownFolder(FOLDERID_StartMenu, "start menu").absolutePath();
}

bool shellDeleteQuiet(const QString &fileName, QWidget *dialog)
{
  if (!QFile::remove(fileName)) {
    return shellDelete(QStringList(fileName), false, dialog);
  }
  return true;
}

QString readFileText(const QString &fileName, QString *encoding)
{
  // the functions from QTextCodec we use are supposed to be reentrant so it's
  // safe to use statics for that
  static QTextCodec *utf8Codec = QTextCodec::codecForName("utf-8");

  QFile textFile(fileName);
  if (!textFile.open(QIODevice::ReadOnly)) {
    return QString();
  }

  QByteArray buffer = textFile.readAll();
  QTextCodec *codec = QTextCodec::codecForUtfText(buffer, utf8Codec);
  QString text = codec->toUnicode(buffer);

  // check reverse conversion. If this was unicode text there can't be data loss
  // this assumes QString doesn't normalize the data in any way so this is a bit unsafe
  if (codec->fromUnicode(text) != buffer) {
    log::debug("conversion failed assuming local encoding");
    codec = QTextCodec::codecForLocale();
    text = codec->toUnicode(buffer);
  }

  if (encoding != nullptr) {
    *encoding = codec->name();
  }

  return text;
}

void removeOldFiles(const QString &path, const QString &pattern, int numToKeep, QDir::SortFlags sorting)
{
  QFileInfoList files = QDir(path).entryInfoList(QStringList(pattern), QDir::Files, sorting);

  if (files.count() > numToKeep) {
    QStringList deleteFiles;
    for (int i = 0; i < files.count() - numToKeep; ++i) {
      deleteFiles.append(files.at(i).absoluteFilePath());
    }

    if (!shellDelete(deleteFiles)) {
      const auto e = ::GetLastError();
      log::warn("failed to remove log files: {}", formatSystemMessage(e));
    }
  }
}


QIcon iconForExecutable(const QString &filePath)
{
  HICON winIcon;
  UINT res = ::ExtractIconExW(ToWString(filePath).c_str(), 0, &winIcon, nullptr, 1);
  if (res == 1) {
    QIcon result = QIcon(QtWin::fromHICON(winIcon));
    ::DestroyIcon(winIcon);
    return result;
  } else {
    return QIcon(":/MO/gui/executable");
  }
}

void deleteChildWidgets(QWidget* w)
{
  auto* ly = w->layout();
  if (!ly) {
    return;
  }

  while (auto* item=ly->takeAt(0)) {
    delete item->widget();
    delete item;
  }
}

std::wstring formatSystemMessage(DWORD id)
{
  wchar_t* message = nullptr;

  const auto ret = FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    id,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    reinterpret_cast<LPWSTR>(&message),
    0, NULL);

  std::wstring s;

  std::wostringstream oss;
  oss << L"0x" << std::hex << id;

  if (ret == 0 || !message) {
    s = oss.str();
  } else {
    s = message;
    boost::trim(s);

    s += L" (" + oss.str() + L")";
  }

  LocalFree(message);

  return s;
}

QString windowsErrorString(DWORD errorCode)
{
  return QString::fromStdWString(formatSystemMessage(errorCode));
}

QString localizedSize(
  unsigned long long bytes,
  const QString& B, const QString& KB, const QString& MB,
  const QString& GB, const QString& TB)
{
  constexpr unsigned long long OneKB = 1024ull;
  constexpr unsigned long long OneMB = 1024ull * 1024;
  constexpr unsigned long long OneGB = 1024ull * 1024 * 1024;
  constexpr unsigned long long OneTB = 1024ull * 1024 * 1024 * 1024;

  auto makeNum = [&](int factor) {
    const double n = bytes / std::pow(1024.0, factor);

    // avoids rounding something like "1.999" to "2.00 KB"
    const double truncated = static_cast<unsigned long long>(n * 100) / 100.0;

    return QString().setNum(truncated, 'f', 2);
  };

  if (bytes < OneKB) {
    return B.arg(bytes);
  } else if (bytes < OneMB) {
    return KB.arg(makeNum(1));
  } else if (bytes < OneGB) {
    return MB.arg(makeNum(2));
  } else if (bytes < OneTB) {
    return GB.arg(makeNum(3));
  } else {
    return TB.arg(makeNum(4));
  }
}


QDLLEXPORT QString localizedByteSize(unsigned long long bytes)
{
  return localizedSize(
    bytes,
    QObject::tr("%1 B"),
    QObject::tr("%1 KB"),
    QObject::tr("%1 MB"),
    QObject::tr("%1 GB"),
    QObject::tr("%1 TB"));
}

QDLLEXPORT QString localizedByteSpeed(unsigned long long bps)
{
  return localizedSize(
    bps,
    QObject::tr("%1 B/s"),
    QObject::tr("%1 KB/s"),
    QObject::tr("%1 MB/s"),
    QObject::tr("%1 GB/s"),
    QObject::tr("%1 TB/s"));
}


QDLLEXPORT void localizedByteSizeTests()
{
  auto f = [](unsigned long long n) {
    return localizedByteSize(n).toStdString();
  };

#define CHECK_EQ(a, b) if ((a) != (b)){ \
  std::cerr << "failed: " << a << " == " << b << "\n"; \
  DebugBreak(); \
}

  CHECK_EQ(f(0),    "0 B");
  CHECK_EQ(f(1),    "1 B");
  CHECK_EQ(f(999),  "999 B");
  CHECK_EQ(f(1000), "1000 B");
  CHECK_EQ(f(1023), "1023 B");

  CHECK_EQ(f(1024),    "1.00 KB");
  CHECK_EQ(f(2047),    "1.99 KB");
  CHECK_EQ(f(2048),    "2.00 KB");
  CHECK_EQ(f(1048575), "1023.99 KB");

  CHECK_EQ(f(1048576),    "1.00 MB");
  CHECK_EQ(f(1073741823), "1023.99 MB");

  CHECK_EQ(f(1073741824), "1.00 GB");
  CHECK_EQ(f(1099511627775), "1023.99 GB");

  CHECK_EQ(f(1099511627776), "1.00 TB");
  CHECK_EQ(f(2759774185818), "2.51 TB");

#undef CHECK_EQ
}

} // namespace MOBase
