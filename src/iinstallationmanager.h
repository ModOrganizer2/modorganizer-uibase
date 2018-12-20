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


#ifndef IINSTALLATIONMANAGER_H
#define IINSTALLATIONMANAGER_H


#include <iplugininstaller.h>

namespace MOBase {


template <typename T> class GuessedValue;


/**
 * @brief The IInstallationManager class
 */
class IInstallationManager {

public:

  virtual ~IInstallationManager() {}

  /**
   * @brief extract the specified file from the currently open archive to a temporary location
   * @param (relative) name of the file within the archive
   * @return the absolute name of the temporary file
   * @note the call will fail with an exception if no archive is open (plugins deriving
   *       from IPluginInstallerSimple can rely on that, custom installers shouldn't)
   * @note the temporary file is automatically cleaned up after the installation
   * @note This call can be very slow if the archive is large and "solid"
   */
  virtual QString extractFile(const QString &fileName) = 0;

  /**
   * @brief extract the specified files from the currently open archive to a temporary location
   * @param files (relative) names of files within the archive
   * @param flatten if true (default) all files go to the same same directory, no subdirectories.
   * @return the absolute names of the temporary files
   * @note the call will fail with an exception if no archive is open (plugins deriving
   *       from IPluginInstallerSimple can rely on that, custom installers shouldn't)
   * @note the temporary file is automatically cleaned up after the installation
   * @note This call can be very slow if the archive is large and "solid"
   */
  virtual QStringList extractFiles(const QStringList &files, bool flatten) = 0;

  /**
   * @brief installs an archive
   * @param modName suggested name of the mod
   * @param archiveFile path to the archive to install
   * @return the installation result
   */
  virtual IPluginInstaller::EInstallResult installArchive(MOBase::GuessedValue<QString> &modName, const QString &archiveFile, const int &modID = 0) = 0;

  /**
   * @brief set the url associated with a mod
   */
  virtual void setURL(QString const &url) = 0;

};

} // namespace MOBase

#endif // IINSTALLATIONMANAGER_H
