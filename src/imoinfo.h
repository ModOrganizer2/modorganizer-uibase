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


#ifndef IMOINFO_H
#define IMOINFO_H


#include "versioninfo.h"
#include "guessedvalue.h"
#include "iprofile.h"
#include <QString>
#include <QStringList>
#include <QVariant>
#include <Windows.h>
#include <functional>

namespace MOBase {

class IModInterface;
class IModRepositoryBridge;
class IDownloadManager;
class IPluginList;
class IModList;
class IPluginGame;

/**
 * @brief Interface to class that provides information about the running session
 *        of Mod Organizer to be used by plugins
 */
class IOrganizer {

public:

  /**
   * @brief information about a virtualised file
   */
  struct FileInfo {
    QString filePath;    /// full path to the file
    QString archive;     /// name of the archive if this file is in a BSA, otherwise this is empty.
    QStringList origins; /// list of origins containing this file. the first one is the highest priority one
  };

public:

  virtual ~IOrganizer() {}

  /**
   * @return create a new nexus interface class
   */
  virtual IModRepositoryBridge *createNexusBridge() const = 0;

  /**
   * @return name of the active profile or an empty string if no profile is loaded (yet)
   */
  virtual QString profileName() const = 0;

  /**
   * @return the (absolute) path to the active profile or an empty string if no profile is loaded (yet)
   */
  virtual QString profilePath() const = 0;

  /**
   * @return the (absolute) path to the download directory
   */
  virtual QString downloadsPath() const = 0;

  /**
   * @return the (absolute) path to the overwrite directory
   */
  virtual QString overwritePath() const = 0;
  
  /**
   * @return the (absolute) path to the base directory
   */
   virtual QString basePath() const =0;

  /**
   * @return the running version of Mod Organizer
   */
  virtual VersionInfo appVersion() const = 0;

  /**
   * @brief retrieve an interface to a mod by its name
   * @param name name of the mod to query
   * @return an interface to the mod or nullptr if there is no mod with the name
   */
  virtual IModInterface *getMod(const QString &name) const = 0;

  /**
   * @brief create a new mod with the specified name
   * @param name name of the new mod
   * @return an interface that can be used to modify the mod. nullptr if the user canceled
   * @note an exception is thrown if the mod already exists. Use "getMod" to verify
   *       the mod-name is unused first
   */
  virtual IModInterface *createMod(GuessedValue<QString> &name) = 0;

  /**
   * @brief get the game plugin matching the specified game
   * @param gameName name of the game short name
   * @return a game plugin, or nullptr if there is no match
   */
  virtual IPluginGame *getGame(const QString &gameName) const = 0;

  /**
   * @brief remove a mod (from disc and from the ui)
   * @param mod the mod to remove
   * @return true on success, false on error
   */
  virtual bool removeMod(IModInterface *mod) = 0;

  /**
   * @brief let the organizer know that a mod has changed
   * @param the mod that has changed
   */
  virtual void modDataChanged(IModInterface *mod) = 0;

  /**
   * @brief retrieve the specified setting for a plugin
   * @param pluginName name of the plugin for which to retrieve a setting. This should always be IPlugin::name() unless you have a really good reason
   *                   to access settings of another mod. You can not access settings for a plugin that isn't installed.
   * @param key identifier of the setting
   * @return the setting
   * @note an invalid qvariant is returned if the setting has not been declared
   */
  virtual QVariant pluginSetting(const QString &pluginName, const QString &key) const = 0;

  /**
   * @brief set the specified setting for a plugin
   * @param pluginName name of the plugin for which to change a value. This should always be IPlugin::name() unless you have a really good reason
   *                   to access data of another mod AND if you can verify that plugin is actually installed
   * @param key identifier of the setting
   * @param value value to set
   * @throw an exception is thrown if pluginName doesn't refer to an installed plugin
   */
  virtual void setPluginSetting(const QString &pluginName, const QString &key, const QVariant &value) = 0;

  /**
   * @brief retrieve the specified persistent value for a plugin
   * @param pluginName name of the plugin for which to retrieve a value. This should always be IPlugin::name() unless you have a really good reason
   *                   to access data of another mod.
   * @param key identifier of the value
   * @param def default value to return if the key is not (yet) set
   * @return the value
   * @note A persistent is an arbitrary value that the plugin can set and retrieve that is persistently stored by the
   *       main application. There is no UI for the user to change this value but (s)he can directly access the storage
   */
  virtual QVariant persistent(const QString &pluginName, const QString &key, const QVariant &def = QVariant()) const = 0;

  /**
   * @brief set the specified persistent value for a plugin
   * @param pluginName name of the plugin for which to change a value. This should always be IPlugin::name() unless you have a really good reason
   *                   to access data of another mod AND if you can verify that plugin is actually installed
   * @param key identifier of the value
   * @param value value to set
   * @param sync if true the storage is immediately written to disc. This costs performance but is safer against data loss
   * @throw an exception is thrown if pluginName doesn't refer to an installed plugin
   */
  virtual void setPersistent(const QString &pluginName, const QString &key, const QVariant &value, bool sync = true) = 0;

  /**
   * @return path to a directory where plugin data should be stored.
   */
  virtual QString pluginDataPath() const = 0;

  /**
   * @brief install a mod archive at the specified location
   * @param fileName absolute file name of the mod to install
   * @param nameSuggestion suggested name for this mod. This can still be changed by the user
   * @return interface to the newly installed mod or nullptr if no installation took place (failure or use canceled
   */
  virtual IModInterface *installMod(const QString &fileName, const QString &nameSuggestion = QString()) = 0;

  /**
   * @brief resolves a path relative to the virtual data directory to its absolute real path
   * @param fileName path to resolve
   * @return the absolute real path or an empty string
   */
  virtual QString resolvePath(const QString &fileName) const = 0;

  /**
   * @brief retrieves a list of (virtual) subdirectories for a path (relative to the data directory)
   * @param directoryName relative path to the directory to list
   * @return a list of directory names
   */
  virtual QStringList listDirectories(const QString &directoryName) const = 0;

  /**
   * @brief find files in the virtual directory matching the filename filter
   * @param path the path to search in
   * @param filter filter function to match against
   * @return a list of matching files
   */
  virtual QStringList findFiles(const QString &path, const std::function<bool(const QString&)> &filter) const = 0;

  /**
   * @brief retrieve the file origins for the speicified file. The origins are listed with their internal name
   * @return list of origins that contain the specified file, sorted by their priority
   * @note the internal name of a mod can differ from the display name for disambiguation
   */
  virtual QStringList getFileOrigins(const QString &fileName) const = 0;

  /**
   * @brief find files in the virtual directory matching the specified complex filter
   * @param path the path to search in
   * @param filter filter function to match against
   * @return a list of matching files
   * @note this function is more expensive than the one filtering by name so use the other one if it suffices
   */
  virtual QList<FileInfo> findFileInfos(const QString &path, const std::function<bool(const FileInfo&)> &filter) const = 0;

  /**
   * @return interface to the download manager
   */
  virtual MOBase::IDownloadManager *downloadManager() const = 0;

  /**
   * @return interface to the list of plugins (esps, esms, and esls)
   */
  virtual MOBase::IPluginList *pluginList() const = 0;

  /**
   * @return interface to the list of mods
   */
  virtual MOBase::IModList *modList() const = 0;

  /**
   * @return interface to the active profile
   */
  virtual MOBase::IProfile *profile() const = 0;

  /**
   * @brief starts an application with virtual filesystem active
   * @param executable name or path of the executable. If this is only a filename it will only work if it has been configured in MO as an executable.
   *        if it is a relative path it is expected to be relative to the game directory.
   * @param args arguments to pass to the executable. If this is empty and executable refers to a configured exe the configured arguments are used
   * @param cwd working directory for the executable. If this is empty the path to the executable is used unless executable referred to a configured
   *        MO executable. In that case the configured cwd is used
   * @param profile profile to use. If this is empty (the default) the current profile is used
   * @return a handle or INVALID_HANDLE_VALUE if the application failed to start.
   * @note the returned handle is usually a job handle but under some situations MO may not be able to create a job and then the handle is a process handle.
   *       Use waitForApplication to wait for completion on this handle, it works for both process and job. If you want to write waiting code yourself, be aware
   *       that you can't WaitForSingleObject on a job object. There is no single windows api call that works for both
   */
  virtual HANDLE startApplication(const QString &executable, const QStringList &args = QStringList(), const QString &cwd = "", const QString &profile = "") = 0;

  /**
   * @brief wait for completion of an application
   * @param handle a process or job handle
   * @param exitCode the exit code of the process after it ended
   * @return true if the application completed, false if waiting failed
   * @note the exit code is slightly fickle if handle is a job handle since there is no reliable way to know for which process the exitcode was returned.
   *       If, at the end of the jobs lifetime only one process was left (for at least 500ms) that processes exitcode will be returned. If there is only
   *       one process, again, the expected exit code is returned. Otherwise,
   */
  virtual bool waitForApplication(HANDLE handle, LPDWORD exitCode = nullptr) const = 0;

  /**
   * @return signal called when a mod has been installed. the parameter to the callback is the mod name
   */
  virtual bool onModInstalled(const std::function<void (const QString &)> &func) = 0;

  /**
   * @param func function to be called when an application is run
   */
  virtual bool onAboutToRun(const std::function<bool(const QString&)> &func) = 0;

  /**
   * @param func function to be called when an application is run
   */
  virtual bool onFinishedRun(const std::function<void(const QString&, unsigned int)> &func) = 0;

  /**
   * @brief refresh the mod list
   * @param saveChanges if true, the relevant profile information is saved first (enabled mods and the ordering)
   */
  virtual void refreshModList(bool saveChanges = true) = 0;

  /**
   * @brief get the currently managed game info
   */
  virtual MOBase::IPluginGame const *managedGame() const = 0;

  /**
   * @brief Get the mod list, sorted by current profile priority
   */
  virtual QStringList modsSortedByProfilePriority() const = 0;
};

} // namespace MOBase

#endif // IMOINFO_H
