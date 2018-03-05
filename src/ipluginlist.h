/*
Mod Organizer shared UI functionality

Copyright (C) 2013 Sebastian Herbord. All rights reserved.

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

#ifndef IPLUGINLIST_H
#define IPLUGINLIST_H

#include <QFlags>

class QString;
class QStringList;

#include <functional>

namespace MOBase {

class IPluginList {

public:

  enum PluginState {
    STATE_MISSING,
    STATE_INACTIVE,
    STATE_ACTIVE
  };

  Q_DECLARE_FLAGS(PluginStates, PluginState)

public:

  virtual ~IPluginList() {}

  /**
   * @return list of plugin names
   */
  virtual QStringList pluginNames() const = 0;

  /**
   * @brief retrieve the state of a plugin
   * @param name filename of the plugin (without path but with file extension)
   * @return one of the possible plugin states: missing, inactive or active
   */
  virtual PluginStates state(const QString &name) const = 0;

  /**
   * @brief set the state of a plugin
   * @param name filename of the plugin (without path but with file extensions)
   * @param state new state of the plugin. should be active or inactive
   */
  virtual void setState(const QString &name, PluginStates state) = 0;

  /**
   * @brief retrieve the priority of a plugin
   * @param name filename of the plugin (without path but with file extension)
   * @return the priority (the higher the more important). Returns -1 if the plugin doesn't exist
   */
  virtual int priority(const QString &name) const = 0;

  /**
   * @brief retrieve the load order of a plugin
   * @param name filename of the plugin (without path but with file extension)
   * @return the load order of a plugin (the order in which the game loads it). If all plugins are enabled this is the same as the priority
   *         but disabled plugins will have a load order of -1. This also returns -1 if the plugin doesn't exist
   */
  virtual int loadOrder(const QString &name) const = 0;

  /**
   * @brief sets the load order of the plugin list.
   * @param the new load order, specified by the list of plugin names, sorted.
   * @note plugins not included in the list will be placed at highest priority
   *       in the order they were before
   */
  virtual void setLoadOrder(const QStringList &pluginList) = 0;

  /**
   * @brief determine if a plugin is a master file (basically a library, referenced by other plugins)
   * @param name filename of the plugin (without path but with file extension)
   * @return true if the file is a master, false if it isn't OR if the file doesn't exist.
   * @note in gamebryo games, a master file will usually have a .esm file
   * extension but technically an esp can be flagged as master and an esm might
   * not be
   */
  virtual bool isMaster(const QString &name) const = 0;

  /**
   * @brief retrieve the list of masters required for this plugin
   * @param name filename of the plugin (without path but with file extension)
   * @return list of masters (filenames with extension, no path)
   */
  virtual QStringList masters(const QString &name) const = 0;

  /**
   * @brief retrieve the name of the origin of a plugin. This is either the (internal!) name of a mod or "overwrite" or "data"
   * @param name filename of the plugin (without path but with file extension)
   * @return name of the origin or an empty string if the plugin doesn't exist
   * @note the internal name of a mod can differ from the display name for disambiguation
   */
  virtual QString origin(const QString &name) const = 0;

  /**
   * @brief invoked whenever the application felt it necessary to refresh the list (i.e. because of external changes)
   */
  virtual bool onRefreshed(const std::function<void()> &callback) = 0;

  /**
   * @brief invoked whenever a plugin has changed priority
   */
  virtual bool onPluginMoved(const std::function<void (const QString &, int, int)> &func) = 0;

  /**
   * @brief invoked whenever a plugin has changed state
   */
  virtual bool onPluginStateChanged(const std::function<void (const QString &, PluginStates)> &func) = 0;

};

}

#endif // IPLUGINLIST_H
