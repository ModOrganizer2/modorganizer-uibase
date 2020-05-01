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


#ifndef IMODINTERFACE_H
#define IMODINTERFACE_H

#include "iplugingame.h"

class QString;
class QStringList;


namespace MOBase {

class VersionInfo;

class IModInterface
{
public:

  virtual ~IModInterface() {}

  /**
   * @return name of the mod
   */
  virtual QString name() const = 0;

  /**
   * @return absolute path to the mod to be used in file system operations
   */
  virtual QString absolutePath() const = 0;

  /**
   * @brief set/change the version of this mod
   * @param version new version of the mod
   */
  virtual void setVersion(const VersionInfo &version) = 0;

  /**
   * @brief sets the installation file for this mod
   * @param fileName archive file name
   */
  virtual void setInstallationFile(const QString &fileName) = 0;

  /**
   * @brief set/change the latest known version of this mod
   * @param version newest known version of the mod
   */
  virtual void setNewestVersion(const VersionInfo &version) = 0;

  /**
   * @brief set endorsement state of the mod
   * @param endorsed new endorsement state
   */
  virtual void setIsEndorsed(bool endorsed) = 0;

  /**
   * @brief sets the mod id on nexus for this mod
   * @param the new id to set
   */
  virtual void setNexusID(int nexusID) = 0;

  /**
   * @brief sets the category id from a nexus category id. Conversion to MO id happens internally
   * @param categoryID the nexus category id
   * @note if a mapping is not possible, the category is set to the default value
   */
  virtual void addNexusCategory(int categoryID) = 0;

  /**
   * @brief assign a category to the mod. If the named category doesn't exist it is created
   * @param categoryName name of the new category
   */
  virtual void addCategory(const QString &categoryName) = 0;

  /**
   * @brief unassign a category from this mod.
   * @param categoryName name of the category to be removed
   * @return true if the category was removed successfully, false if no such category was assigned
   */
  virtual bool removeCategory(const QString &categoryName) = 0;

  /**
   * @return list of categories assigned to this mod
   */
  virtual QStringList categories() const = 0;

  /**
  * @brief set/change the game plgin for this mod
  *
  * @param gamePlugin the game plugin
  */
  virtual void setGamePlugin(const IPluginGame *gamePlugin) = 0;

  /**
   * @brief set the name of this mod
   *
   * set the name of this mod. This will also update the name of the
   * directory that contains this mod
   *
   * @param name new name of the mod
   * @return true on success, false if the new name can't be used (i.e. because the new
   *         directory name wouldn't be valid)
   **/
  virtual bool setName(const QString &name) = 0;

  /**
   * @brief delete the mod from the disc. This does not update the global ModInfo structure or indices
   * @return true on success
   */
  virtual bool remove() = 0;

};


} // namespace MOBase

#endif // IMODINTERFACE_H
