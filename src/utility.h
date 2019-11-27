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

#ifndef UTILITY_H
#define UTILITY_H

#include "dllimport.h"
#include <vector>
#include <set>
#include <algorithm>
#include <QString>
#include <QTextStream>
#include <QDir>
#include <QIcon>
#include <QUrl>
#include <Windows.h>
#include <ShlObj.h>


namespace MOBase {

/**
 * @brief remove the specified directory including all sub-directories
 *
 * @param dirName name of the directory to delete
 * @return true on success. in case of an error, "removeDir" itself displays an error message
 **/
QDLLEXPORT bool removeDir(const QString &dirName);

/**
 * @brief copy a directory recursively
 * @param sourceName name of the directory to copy
 * @param destinationName name of the target directory
 * @param merge if true, the destination directory is allowed to exist, files will then
 *              be added to that directory. If false, the call will fail in that case
 * @return true if files were copied. This doesn't necessary mean ALL files were copied
 * @note symbolic links are not followed to prevent endless recursion
 */
QDLLEXPORT bool copyDir(const QString &sourceName, const QString &destinationName, bool merge);

/**
 * @brief move a file, creating subdirectories as needed
 * @param source source file name
 * @param destination destination file name
 * @return true if the file was successfully copied
 */
QDLLEXPORT bool moveFileRecursive(const QString &source, const QString &baseDir, const QString &destination);

/**
 * @brief copy a file, creating subdirectories as needed
 * @param source source file name
 * @param destination destination file name
 * @return true if the file was successfully copied
 */
QDLLEXPORT bool copyFileRecursive(const QString &source, const QString &baseDir, const QString &destination);

/**
 * @brief copy one or multiple files using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param sourceNames names of files to be copied. This can include wildcards
 * @param destinationNames names of the files in the destination location or the destination directory to copy to.
 *                         There has to be one destination name for each source name or a single directory
 * @param dialog a dialog to be the parent of possible confirmation dialogs
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellCopy(const QStringList &sourceNames, const QStringList &destinationNames, QWidget *dialog = nullptr);

/**
 * @brief copy one or multiple files using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param sourceName names of file to be copied. This can include wildcards
 * @param destinationName name of the files in the destination location or the destination directory to copy to.
 *                        There has to be one destination name for each source name or a single directory
 * @param yesToAll if true, the operation will assume "yes" to overwrite confirmations. This doesn't seem to work when providing
 *                 multiple files to copy
 * @param dialog a dialog to be the parent of possible confirmation dialogs
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellCopy(const QString &sourceNames, const QString &destinationNames, bool yesToAll = false, QWidget *dialog = nullptr);

/**
 * @brief move one or multiple files using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param sourceNames names of files to be moved. This can include wildcards
 * @param destinationNames names of the files in the destination location or the destination directory to move to.
 *                         There has to be one destination name for each source name or a single directory
 * @param dialog a dialog to be the parent of possible confirmation dialogs
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellMove(const QStringList &sourceNames, const QStringList &destinationNames, QWidget *dialog = nullptr);

/**
 * @brief move one files using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param sourceNames names of files to be moved. This can include wildcards
 * @param destinationNames names of the files in the destination location or the destination directory to move to.
 *                         There has to be one destination name for each source name or a single directory
 * @param dialog a dialog to be the parent of possible confirmation dialogs
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellMove(const QString &sourceNames, const QString &destinationNames, bool yesToAll = false, QWidget *dialog = nullptr);

/**
 * @brief rename a file using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param oldName old name of file to be renamed
 * @param newName new name of the file
 * @param dialog a dialog to be the parent of possible confirmation dialogs
 * @param yesToAll if true, the operation will assume "yes" to all overwrite confirmations
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellRename(const QString &oldName, const QString &newName, bool yesToAll = false, QWidget *dialog = nullptr);

/**
 * @brief delete files using a shell operation (this will ask the user for confirmation on overwrite
 *        or elevation requirement)
 * @param fileNames names of files to be deleted
 * @param recycle if true, the file goes to the recycle bin instead of being permanently deleted
 * @return true on success, false on error. Call ::GetLastError() to retrieve error code
 **/
QDLLEXPORT bool shellDelete(const QStringList &fileNames, bool recycle = false, QWidget *dialog = nullptr);

/**
 * @brief delete a file. This tries a regular delete and falls back to a shell operation if that fails.
 * @param fileName names of file to be deleted
 * @note this is a workaround for win 8 and newer where shell operations caused the windows to loose focus even if no dialog is shown
 **/
QDLLEXPORT bool shellDeleteQuiet(const QString &fileName, QWidget *dialog = nullptr);


namespace shell
{
  namespace details
  {
    // used by HandlePtr, calls CloseHandle() as the deleter
    //
    struct HandleCloser
    {
      using pointer = HANDLE;

      void operator()(HANDLE h)
      {
        if (h != INVALID_HANDLE_VALUE) {
          ::CloseHandle(h);
        }
      }
    };

    using HandlePtr = std::unique_ptr<HANDLE, HandleCloser>;
  }


  // returned by the various shell functions; note that the process handle is
  // closed in the destructor, unless stealProcessHandle() was called
  //
  class QDLLEXPORT Result
  {
  public:
    Result(bool success, DWORD error, QString message, HANDLE process);

    // non-copyable
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
    Result(Result&&) = default;
    Result& operator=(Result&&) = default;

    static Result makeFailure(DWORD error, QString message={});
    static Result makeSuccess(HANDLE process=INVALID_HANDLE_VALUE);

    // whether the operation was successful
    //
    bool success() const;
    explicit operator bool() const;

    // error returned by the underlying function
    //
    DWORD error();

    // string representation of the message, may be localized
    //
    const QString& message() const;

    // process handle, if any
    //
    HANDLE processHandle() const;

    // process handle, if any; sets the internal handle to INVALID_HANDLE_VALUE
    // so that the caller is in charge of closing it
    //
    HANDLE stealProcessHandle();

    // the message, or the error number if empty
    //
    QString toString() const;

  private:
    bool m_success;
    DWORD m_error;
    QString m_message;
    details::HandlePtr m_process;
  };

  // returns a string representation of the given shell error; these errors are
  // returned as an HINSTANCE from various functions such as ShellExecuteW() or
  // FindExecutableW()
  //
  QDLLEXPORT QString formatError(int i);


  // starts explorer using the given directory and/or file
  //
  // if `info` is a directory, opens it in explorer; if it's a file, opens the
  // directory and selects it
  //
  QDLLEXPORT Result Explore(const QFileInfo& info);

  // starts explorer using the given directory and/or file
  //
  // if `path` is a directory, opens it in explorer; if it's a file, opens the
  // directory and selects it
  //
  QDLLEXPORT Result Explore(const QString& path);

  // starts explorer using the given directory
  //
  QDLLEXPORT Result Explore(const QDir& dir);


  // asks the shell to open the given file with its default handler
  //
  QDLLEXPORT Result Open(const QString& path);

  // asks the shell to open the given link with the default browser
  //
  QDLLEXPORT Result Open(const QUrl& url);


  // @brief asks the shell to execute the given program, with optional
  // parameters
  //
  QDLLEXPORT Result Execute(const QString& program, const QString& params={});


  // asks the shell to delete the given file (not directory)
  //
  QDLLEXPORT Result Delete(const QFileInfo& path);

  // asks the shell to rename a file or directory from `src` to `dest`; works
  // across volumes
  //
  QDLLEXPORT Result Rename(const QFileInfo& src, const QFileInfo& dest);
}

/**
 * @brief construct a string containing the elements of a vector concatenated
 *
 * @param value the container to concatenate
 * @param separator sperator to put between elements
 * @param maximum maximum number of elements to print. If there are more elements, "..." is appended to the string Defaults to UINT_MAX.
 * @return a string containing up to "maximum" elements from "value" separated by "separator"
 **/
template <typename T>
QString VectorJoin(const std::vector<T> &value, const QString &separator, size_t maximum = UINT_MAX)
{
  QString result;
  if (value.size() != 0) {
    QTextStream stream(&result);
    stream << value[0];
    for (unsigned int i = 1; i < (std::min)(value.size(), maximum); ++i) {
      stream << separator << value[i];
    }
    if (maximum < value.size()) {
      stream << separator << "...";
    }
  }
  return result;
}


/**
 * @brief construct a string containing the elements of a std::set concatenated
 *
 * @param value the container to concatenate
 * @param separator sperator to put between elements
 * @param maximum maximum number of elements to print. If there are more elements, "..." is appended to the string Defaults to UINT_MAX.
 * @return a string containing up to "maximum" elements from "value" separated by "separator"
 **/
template <typename T>
QString SetJoin(const std::set<T> &value, const QString &separator, size_t maximum = UINT_MAX)
{
  QString result;
  typename std::set<T>::const_iterator iter = value.begin();
  if (iter != value.end()) {
    QTextStream stream(&result);
    stream << *iter;
    ++iter;
    unsigned int pos = 1;
    for (; iter != value.end() && pos < maximum; ++iter) {
      stream << separator << *iter;
    }
    if (maximum < value.size()) {
      stream << separator << "...";
    }
  }
  return result;
}


/**
 * @brief exception class that takes a QString as the parameter
 **/

#pragma warning(push)
#pragma warning(disable: 4275)  // non-dll interface

class QDLLEXPORT MyException : public std::exception {
public:
  /**
   * @brief constructor
   *
   * @param text exception text
   **/
  MyException(const QString &text);

  virtual const char* what() const throw()
          { return m_Message.constData(); }
private:
  QByteArray m_Message;
};

#pragma warning(pop)


/**
 * @brief exception thrown in case of incompatibilities, i.e. between plugins
 */
class QDLLEXPORT IncompatibilityException : public MyException {
public:
  IncompatibilityException(const QString &text) : MyException(text) {}
};


/**
 * @brief convenience template to create a vector in a single call using vararg semantic
 *
 * @param count number of elements
 * @param  ... parameters (should be exactly "count" elements)
 * @return the constructed vector
 **/
template <typename T>
std::vector<T> MakeVector(int count, ...)
{
  std::vector<T> result;
  va_list argList;
  va_start(argList, count);
  for (int i = 0; i < count; ++i) {
    result.push_back(va_arg(argList, int));
  }
  va_end(argList);
  return result;
}

template <typename T>
QList<T> ConvertList(const QVariantList &variants)
{
  QList<T> result;
  foreach (QVariant var, variants) {
    if (!var.canConvert<T>()) {
      throw MyException("invalid variant type");
    }
    result.append(var.value<T>());
  }
}

/**
 * @brief convert QString to std::wstring (utf-16 encoding)
 **/
QDLLEXPORT std::wstring ToWString(const QString &source);

/**
 * @brief convert QString to std::string
 * @param source source string
 * @param utf8 if true, the output string is utf8, otherwise it's the local 8bit encoding (according to qt)
 **/
QDLLEXPORT std::string ToString(const QString &source, bool utf8 = true);

/**
 * @brief convert std::string to QString (assuming the string to be utf-8 encoded)
 **/
QDLLEXPORT QString ToQString(const std::string &source);

/**
 * @brief convert std::wstring to QString (assuming the wstring to be utf-16 encoded)
 **/
QDLLEXPORT QString ToQString(const std::wstring &source);

/**
 * @brief convert a systemtime object to a string containing date and time in local representation
 *
 * @param time the time to convert
 * @return string representation of the time object
 **/
QDLLEXPORT QString ToString(const SYSTEMTIME &time);

/**
 * throws on failure
 * @param id    the folder id
 * @param what  the name of the folder, used for logging errors only
 * @return absolute path of the given known folder id
 **/
QDLLEXPORT QDir getKnownFolder(KNOWNFOLDERID id, const QString& what={});

/**
 * throws on failure
 * @return absolute path of the desktop directory for the current user
 **/
QDLLEXPORT QString getDesktopDirectory();

/**
* throws on failure
* @return absolute path of the start menu directory for the current user
 **/
QDLLEXPORT QString getStartMenuDirectory();

/**
 * @brief fix a directory name so it can be dealt with by windows explorer
 * @return false if there was no way to convert the name into a valid one
 **/
QDLLEXPORT bool fixDirectoryName(QString &name);

/**
 * @brief read a file and return it's content as a unicode string. This tries to guess
 *        the encoding used in the file
 * @param fileName name of the file to read
 * @param encoding (optional) if this is set, the target variable received the name of the encoding used
 * @return the textual content of the file or an empty string if the file doesn't exist
 **/
QDLLEXPORT QString readFileText(const QString &fileName, QString *encoding = nullptr);

/**
 * @brief delete files matching a pattern
 * @param directory in which to delete files
 * @param pattern the name pattern files have to match
 * @param numToKeep the number of files to keep
 * @param sorting if numToKeep is not 0, the last numToKeep files according to this sorting a kept
 **/
QDLLEXPORT void removeOldFiles(const QString &path, const QString &pattern, int numToKeep, QDir::SortFlags sorting = QDir::Time);

/**
 * @brief retrieve the icon of an executable. Currently this always extracts the biggest icon
 * @param absolute path to the executable
 * @return the icon
 **/
QDLLEXPORT QIcon iconForExecutable(const QString &filePath);

// removes and deletes all the children of the given widget
//
QDLLEXPORT void deleteChildWidgets(QWidget* w);

template <typename T>
bool isOneOf(const T &val, const std::initializer_list<T> &list) {
  return std::find(list.begin(), list.end(), val) != list.end();
}

QDLLEXPORT std::wstring formatSystemMessage(DWORD id);

inline std::wstring formatSystemMessage(HRESULT hr)
{
  return formatSystemMessage(static_cast<DWORD>(hr));
}


// forwards to formatSystemMessage(), preserved for ABI
//
QDLLEXPORT QString windowsErrorString(DWORD errorCode);


QDLLEXPORT QString localizedByteSize(unsigned long long bytes);
QDLLEXPORT QString localizedByteSpeed(unsigned long long bytesPerSecond);

template <class F>
class Guard
{
public:
  Guard()
    : m_call(false)
  {
  }

  Guard(F f)
    : m_f(f), m_call(true)
  {
  }

  Guard(Guard&& g)
    : m_f(std::move(g.m_f))
  {
    g.m_call = false;
  }

  ~Guard()
  {
    if (m_call)
      m_f();
  }

  Guard& operator=(Guard&& g)
  {
    m_f = std::move(g.m_f);
    g.m_call = false;
    return *this;
  }


  void kill()
  {
    m_call = false;
  }


  Guard(const Guard&) = delete;
  Guard& operator=(const Guard&) = delete;

private:
  F m_f;
  bool m_call;
};

} // namespace MOBase

#endif // UTILITY_H
