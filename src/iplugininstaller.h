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


#ifndef IPLUGININSTALLER_H
#define IPLUGININSTALLER_H


#include "iplugin.h"
#include "ifiletree.h"
#include <set>

namespace MOBase {


class IInstallationManager;


class IPluginInstaller : public IPlugin {

  Q_INTERFACES(IPlugin)

public:

  enum EInstallResult {
    RESULT_SUCCESS,
    RESULT_FAILED,
    RESULT_CANCELED,
    RESULT_MANUALREQUESTED,
    RESULT_NOTATTEMPTED,
    RESULT_SUCCESSCANCEL
  };

public:

  IPluginInstaller() : m_ParentWidget(nullptr), m_InstallationManager(nullptr) {}

  /**
   * retrieve the priority of this installer. If multiple installers are able
   * to handle an archive, the one with the highest priority wins.
   * @return the priority of the installer
   */
  virtual unsigned int priority() const = 0;

  /**
   * @return true if this plugin should be treated as a manual installer if the user
   * explicitly requested one. A manual installer should offer the user maximum amount of
   * customizability
   */
  virtual bool isManualInstaller() const = 0;

  /**
   * @brief test if the archive represented by the tree parameter can be installed through this installer
   * @param tree a directory tree representing the archive
   * @return true if this installer can handle the archive
   */
  virtual bool isArchiveSupported(std::shared_ptr<const IFileTree> tree) const = 0;

  /**
   * @brief sets the widget that the tool should use as the parent whenever
   *        it creates a new modal dialog
   * @param widget the new parent widget
   */
  virtual void setParentWidget(QWidget *widget) { m_ParentWidget = widget; }

  /**
   * brief sets the installation manager responsible for the installation process
   * it can be used by plugins to access utility functions
   * @param manager the new installation manager
   */
  void setInstallationManager(IInstallationManager *manager) { m_InstallationManager = manager; }

protected:

  QWidget *parentWidget() const { return m_ParentWidget; }
  IInstallationManager *manager() const { return m_InstallationManager; }

private:

  QWidget *m_ParentWidget;
  IInstallationManager *m_InstallationManager;

};

} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginInstaller, "com.tannin.ModOrganizer.PluginInstaller/1.0")

#endif // IPLUGININSTALLER_H
