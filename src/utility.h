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
#ifndef WIN32_MEAN_AND_LEAN
#define WIN32_MEAN_AND_LEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>


namespace MOBase {

QDLLEXPORT QString windowsErrorString(DWORD errorCode);

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
 * @return absolute path of the the desktop directory for the current user
 **/
QDLLEXPORT QString getDesktopDirectory();

/**
 * @return absolute path of the the start menu directory for the current user
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

template <typename T>
bool isOneOf(const T &val, const std::initializer_list<T> &list) {
  return std::find(list.begin(), list.end(), val) != list.end();
}

} // namespace MOBase

#endif // UTILITY_H
