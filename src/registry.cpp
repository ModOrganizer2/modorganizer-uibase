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
          if (QMessageBox::question(QApplication::activeModalWidget(),QApplication::tr("INI file is read-only"),
                QApplication::tr("Mod Organizer is attempting to write to \"%1\" which is currently set to read-only. "
                "Clear the read-only flag to allow the write?").arg(fileName)) == QMessageBox::Yes) {
            log::warn("{} is read-only. Attempting to clear read-only flag.", QString::fromWCharArray(fileName));
            attrs &= ~(FILE_ATTRIBUTE_READONLY);
            if (::SetFileAttributes(fileName, attrs)) {
              if (::WritePrivateProfileString(appName, keyName, value, fileName)) {
                success = true;
              }
            }
          } else {
            log::warn("{} is read-only. User denied clearing the read-only flag.", QString::fromWCharArray(fileName));
          }
        }
      } break;
    }
  }

  return success;
}

} // namespace MOBase
