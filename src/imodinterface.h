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

#include <utility>
#include <set>

#include <QColor>
#include <QDateTime>
#include <QString>
#include <QStringList>


namespace MOBase {

class VersionInfo;

enum class EndorsedState {
  ENDORSED_FALSE,
  ENDORSED_TRUE,
  ENDORSED_UNKNOWN,
  ENDORSED_NEVER
};

enum class TrackedState {
  TRACKED_FALSE,
  TRACKED_TRUE,
  TRACKED_UNKNOWN,
};

class IModInterface
{
public:

  virtual ~IModInterface() {}

public: // Non-meta related information:

  /**
   * @return name of the mod
   */
  virtual QString name() const = 0;

  /**
   * @return absolute path to the mod to be used in file system operations
   */
  virtual QString absolutePath() const = 0;

public: // Meta-related information:

  virtual QString comments() const = 0;

  virtual QString notes() const = 0;

  virtual QString gameName() const = 0;

  virtual int modId() const = 0;

  virtual VersionInfo version() const = 0;
  virtual VersionInfo newestVersion() const = 0;
  virtual VersionInfo ignoredVersion() const = 0;

  virtual QString installationFile() const = 0;
  virtual std::set<std::pair<int, int>> installedFiles() const = 0;

  virtual QString repository() const = 0;

  // virtual bool converted() const = 0;
  // virtual bool validated() const = 0;

  virtual QColor color() const = 0;

  /**
   * @return the URL of this mod, or an empty QString() if no URL is associated
   *     with this mod.
   */
  virtual QString url() const = 0;

  // Nexus-related, do not expose?
  // virtual QString nexusDescription() const = 0;
  // virtual int nexusFileStatus() const = 0;
  // virtual QDateTime lastNexusQuery() const = 0;
  // virtual QDateTime lastNexusUpdate() const = 0;
  // virtual QDateTime lastNexusModified() const = 0;

  virtual int primaryCategory() const = 0;
  virtual QStringList categories() const = 0;

  virtual TrackedState trackedState() const = 0;
  virtual EndorsedState endorsedState() const = 0;

public: // Mutable operations:

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
   * @brief set/change the source game of this mod
   *
   * @param gameName the source game shortName
   */
  virtual void setGameName(const QString &gameName) = 0;

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
