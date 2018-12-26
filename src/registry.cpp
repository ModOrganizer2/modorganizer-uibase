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

#include <QString>

namespace MOBase {

bool WriteRegistryValue(LPCWSTR appName, LPCWSTR keyName, LPCWSTR value, LPCWSTR fileName)
{
  bool success = true;
  if (!::WritePrivateProfileString(appName, keyName, value, fileName)) {
    success = false;
    switch(::GetLastError()) {
      case ERROR_ACCESS_DENIED:
      {
        DWORD attrs = GetFileAttributes(fileName);
        if ((attrs != INVALID_FILE_ATTRIBUTES) && (attrs & FILE_ATTRIBUTE_READONLY)) {
          qWarning(QString("%1 is read-only.  Attempting to fix that...").arg(fileName).toLocal8Bit());
          attrs &= ~(FILE_ATTRIBUTE_READONLY);
          if (SetFileAttributes(fileName, attrs)) {
            if (WritePrivateProfileString(appName, keyName, value, fileName)) {
              success = true;
            }
          }
        }
      } break;
    }
  }

  return success;
}

} // namespace MOBase