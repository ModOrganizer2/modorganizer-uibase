/*
This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "registry.h"
#include "log.h"
#include "report.h"
#include <QString>
#include <QMessageBox>
#include <QApplication>

namespace MOBase {

bool WriteRegistryValue(LPCWSTR appName, LPCWSTR keyName, LPCWSTR value, LPCWSTR fileName)
{
  bool success = true;
  if (!::WritePrivateProfileString(appName, keyName, value, fileName)) {
    success = false;
    switch(::GetLastError()) {
      case ERROR_ACCESS_DENIED:
      {
        DWORD attrs = ::GetFileAttributes(fileName);
        if ((attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_READONLY)) {
          QFileInfo fileInfo(QString("%1").arg(fileName));

          QMessageBox::StandardButton result =
            MOBase::TaskDialog(
              qApp->activeModalWidget(),
              QObject::tr("INI file is read-only"))
            .main(QObject::tr("INI file is read-only"))
            .content(QObject::tr("Mod Organizer is attempting to write to \"%1\" which is currently set to read-only.").arg(fileInfo.fileName()))
            .icon(QMessageBox::Warning)
            .button({
              QObject::tr("Clear the read-only flag"),
              QMessageBox::Yes})
            .button({
              QObject::tr("Allow the write once"),
              QObject::tr("The file will be set to read-only again."),
              QMessageBox::Ignore})
            .button({
              QObject::tr("Skip this file"),
              QMessageBox::No})
            .remember("clearReadOnly",fileInfo.fileName())
            .exec();

          // clear the read-only flag if requested
          if (result & (QMessageBox::Yes |QMessageBox::Ignore)) {
            attrs &= ~(FILE_ATTRIBUTE_READONLY);
            if (::SetFileAttributes(fileName, attrs)) {
              if (::WritePrivateProfileString(appName, keyName, value, fileName)) {
                success = true;
              }
            }
          }

          // set the read-only flag if requested
          if (result == QMessageBox::Ignore) {
            attrs |= FILE_ATTRIBUTE_READONLY;
            ::SetFileAttributes(fileName, attrs);
          }
        }
      } break;
    }
  }

  return success;
}

} // namespace MOBase
