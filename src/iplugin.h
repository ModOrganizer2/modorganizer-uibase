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


#ifndef IPLUGIN_H
#define IPLUGIN_H


#include "versioninfo.h"
#include "imoinfo.h"
#include "pluginsetting.h"
#include <QString>
#include <QObject>

namespace MOBase {


class IPlugin {
public:
  virtual ~IPlugin() {}

  /** initialize the plugin. This is called immediately after loading the plugin,
   *  no other function is called before. Plugins will probably want to store the
   *  moInfo pointer. It is guaranteed to be valid as long as the plugin is loaded.
   *  return false if the plugin can't be initialized, i.e. because the version
   *  is invalid. */
  virtual bool init(IOrganizer *moInfo) = 0;

  /**
   * @return name of this plugin (used for example in the settings menu)
   * @note Please ensure you use a name that will not change. Do NOT include a version number in the name.
   *       Do NOT use a localizable string (tr()) here
   * Settings for example are tied to this name, if you rename your plugin you lose settings users made
   */
  virtual QString name() const = 0;

  /**
   * @return author of this plugin
   */
  virtual QString author() const = 0;

  /**
   * @return a short description of the plugin to be displayed to the user
   */
  virtual QString description() const = 0;

  /**
   * @return version of the plugin. This can be used to detect outdated versions of plugins
   */
  virtual VersionInfo version() const = 0;

  /**
   * @brief called to test if this plugin is active. inactive plugins can still be configured
   *        and report problems but otherwise have no effect
   * @return true if this plugin is active
   */
  virtual bool isActive() const = 0;

  /**
   * @return list of configurable settings for this plugin. The list may be empty
   */
  virtual QList<PluginSetting> settings() const = 0;

};

} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPlugin, "com.tannin.ModOrganizer.Plugin/2.0")

#endif // IPLUGIN_H
