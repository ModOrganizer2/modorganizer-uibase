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

#include <QDir>
#include <QList>
#include <QString>

#include "extension.h"

namespace MOBase
{

class IPluginLoader : public QObject
{
public:
  // initialize the loader, set the error message on failure
  //
  virtual bool initialize(QString& errorMessage) = 0;

  // extract and load plugins from the given extension
  //
  // if multiple QObject* corresponds to the same Plugin, they should be returned
  // together
  //
  virtual QList<QList<QObject*>> load(const PluginExtension& extension) = 0;

  // unload plugins from the given extension
  //
  virtual void unload(const PluginExtension& identifier) = 0;

  // unload all plugins from this loader
  //
  virtual void unloadAll() = 0;

  virtual ~IPluginLoader() {}

protected:
  IPluginLoader() {}
};

}  // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginLoader, "com.mo2.PluginLoader")

#endif  // IPLUGINPROXY_H
