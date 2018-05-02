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

#ifndef IPLUGINPROXY_H
#define IPLUGINPROXY_H

#include "iplugin.h"


namespace MOBase {

class IPluginProxy : public IPlugin {
public:

  IPluginProxy() : m_ParentWidget(nullptr) {}

  /**
   * @param pluginPath path to plugins
   * @return list of plugins that supported by this proxy
   */
  virtual QStringList pluginList(const QString &pluginPath) const = 0;

  /**
   * @brief instantiate a proxied plugin
   * @param pluginName name of the proxied plugin to instantiate
   * @return plugin object
   */
  virtual QList<QObject*> instantiate(const QString &pluginName) = 0;

  /**
   * @brief sets the widget that the tool should use as the parent whenever
   *        it creates a new modal dialog
   * @param widget the new parent widget
   */
  void setParentWidget(QWidget *widget) { m_ParentWidget = widget; }

protected:

  QWidget *parentWidget() const { return m_ParentWidget; }

private:

  QWidget *m_ParentWidget;

};


} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginProxy, "com.tannin.ModOrganizer.PluginProxy/1.0")

#endif // IPLUGINPROXY_H
