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

#ifndef IINSTANCEMANAGER_H
#define IINSTANCEMANAGER_H

#include <QString>

#include <memory>
#include <vector>

namespace MOBase
{
class IInstance;

/**
 * @brief Interface to the instance manager of Mod Organizer.
 */
class IInstanceManager
{
public:
  /**
   * @brief Override the instance name found in the registry.
   */
  virtual void overrideInstance(const QString& instanceName) = 0;

  /**
   * @brief Override the profile found in the INI file of the current instance.
   */
  virtual void overrideProfile(const QString& profileName) = 0;

  /**
   * @brief Clear the instance and profile overrides (used when restarting Mod Organizer
   * to select another instance).
   */
  virtual void clearOverrides() = 0;

  /**
   * @brief Clear the instance name from the registry.
   *
   * @note On restart, this will make Mod Organizer either select the portable instance
   * if it exists, or display the instance selection/creation dialog.
   */
  virtual void clearCurrentInstance() = 0;

  /**
   * @return the current instance.
   */
  virtual std::shared_ptr<IInstance> currentInstance() const = 0;

  /**
   * @brief Set the instance name in the registry so it will be opened the next time Mod
   * Organizer is started.
   *
   * @param instanceName Name of the instance to set as current.
   */
  virtual void setCurrentInstance(const QString& instanceName) = 0;

  /**
   * @return true if the user can change the current instance from the user interface.
   */
  virtual bool allowedToChangeInstance() const = 0;

  /**
   * @return true if a portable instance exists.
   */
  virtual bool portableInstanceExists() const = 0;

  /**
   * @return the absolute path to the portable instance, regardless of whether one
   * exists.
   */
  virtual QString portablePath() const = 0;

  /**
   * @return the absolute path to the directory that contains global instances.
   *
   * @note This is typically "%LOCALAPPDATA%\ModOrganizer".
   */
  virtual QString globalInstancesRootPath() const = 0;

  /**
   * @return The list of absolute paths to all global instances.
   *
   * @note This does not include the portable instance.
   */
  virtual std::vector<QString> globalInstancePaths() const = 0;

  /**
   * @brief Check if a global instance with the specified name exists.
   *
   * @param instanceName Name of the global instance to check.
   *
   * @return true if a global instance with the specified name exists, false otherwise.
   */
  virtual bool globalInstanceExists(const QString& instanceName) const = 0;

  /**
   * @brief Retrieve the absolute path to the specified global instance.
   *
   * @param instanceName Name of the global instance.
   *
   * @return the absolute path to the specified global instance.
   */
  virtual QString globalInstancePath(const QString& instanceName) const = 0;

  /**
   * @brief Retrieve the absolute path to the INI file for the specified instance
   * directory.
   *
   * @param instanceDirectory The instance directory.
   *
   * @return the absolute path to the INI file of the specified instance.
   *
   * @note The file may not exist.
   */
  virtual QString iniPath(const QString& instanceDirectory) const = 0;
};
}  // namespace MOBase

#endif  // IINSTANCEMANAGER_H
