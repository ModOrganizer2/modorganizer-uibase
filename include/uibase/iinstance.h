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

#ifndef IINSTANCE_H
#define IINSTANCE_H

#include <QString>

namespace MOBase
{
class IPluginGame;

/**
 * @brief Represents a Mod Organizer instance, either global or portable.
 */
class IInstance
{
public:
  /**
   * @brief Read instance data from the INI file if not yet known.
   *
   * @note This includes the game name, game directory, base directory, and profile
   * name.
   *
   * @return true on success, false on failure (e.g. INI file not found or invalid).
   */
  virtual bool readFromIni() = 0;

  /**
   * @return The instance name; this is the directory name or "Portable" for portable
   * instances.
   */
  virtual QString displayName() const = 0;

  /**
   * @return The name of the game managed by this instance, or an empty string if not
   * yet read from the INI file.
   *
   * @note Only available after readFromIni() has been called successfully.
   */
  virtual QString gameName() const = 0;

  /**
   * @return The directory where the game is installed, or an empty string if not yet
   * read from the INI file.
   *
   * @note Only available after readFromIni() has been called successfully.
   */
  virtual QString gameDirectory() const = 0;

  /**
   * @return The instance directory.
   */
  virtual QString directory() const = 0;

  /**
   * @return The base directory configured for this instance, or an empty string if not
   * yet read from the INI file.
   *
   * @note Only available after readFromIni() has been called successfully.
   */
  virtual QString baseDirectory() const = 0;

  /**
   * @return true if this is a portable instance, false if it is a global one.
   */
  virtual bool isPortable() const = 0;

  /**
   * @return The name of the profile currently selected in this instance, or an empty
   * string if not yet read from the INI file.
   *
   * @note Only available after readFromIni() has been called successfully.
   */
  virtual QString profileName() const = 0;

  /**
   * @return The path to the INI file used by this instance.
   *
   * @note This file may not exist.
   */
  virtual QString iniPath() const = 0;
};
}  // namespace MOBase

#endif  // IINSTANCE_H
