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
          QMessageBox::StandardButton result =
            MOBase::TaskDialog(
              qApp->activeModalWidget(),
              qApp->tr("INI file is read-only"))
            .main(qApp->tr("INI file is read-only"))
            .content(qApp->tr("Mod Organizer is attempting to write to \"%1\" which is currently set to read-only. "
              "Clear the read-only flag to allow the write?").arg(fileName))
            .icon(QMessageBox::Question)
            .button({
              qApp->tr("Clear the read-only flag"),
              QMessageBox::Yes})
            .button({
              qApp->tr("Do nothing"),
              qApp->tr("The write operation may fail."),
              QMessageBox::Cancel})
            .remember("clearReadOnly", QString("%1").arg(fileName))
            .exec();

          if (result == QMessageBox::Yes) {
            attrs &= ~(FILE_ATTRIBUTE_READONLY);
            if (::SetFileAttributes(fileName, attrs)) {
              if (::WritePrivateProfileString(appName, keyName, value, fileName)) {
                success = true;
              }
            }
          } 
        }
      } break;
    }
  }

  return success;
}

} // namespace MOBase
