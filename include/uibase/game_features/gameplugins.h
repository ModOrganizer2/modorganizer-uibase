#ifndef UIBASE_GAMEFEATURES_GAMEPLUGINS_H
#define UIBASE_GAMEFEATURES_GAMEPLUGINS_H

#include <QStringList>

#include "./game_feature.h"

namespace MOBase
{
class IPluginList;

class GamePlugins : public details::GameFeatureCRTP<GamePlugins>
{
public:
  virtual void writePluginLists(const MOBase::IPluginList* pluginList) = 0;
  virtual void readPluginLists(MOBase::IPluginList* pluginList)        = 0;
  virtual QStringList getLoadOrder()                                   = 0;
  virtual bool lightPluginsAreSupported()                              = 0;
  virtual bool mediumPluginsAreSupported()                             = 0;
};

}  // namespace MOBase

#endif  // GAMEPLUGINS_H
