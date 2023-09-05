#ifndef GAMEPLUGINS_H
#define GAMEPLUGINS_H

#include <QStringList>

namespace MOBase
{
class IPluginList;
}

class GamePlugins
{

public:
  virtual ~GamePlugins() {}

  virtual void writePluginLists(const MOBase::IPluginList* pluginList) = 0;
  virtual void readPluginLists(MOBase::IPluginList* pluginList)        = 0;
  virtual QStringList getLoadOrder()                                   = 0;
  virtual bool lightPluginsAreSupported()                              = 0;
  virtual bool overridePluginsAreSupported()                           = 0;
};

#endif  // GAMEPLUGINS_H
