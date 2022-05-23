#include "extension.h"

#include <QFile>
#include <QFileInfo>

namespace MOBase
{

IExtension::IExtension(std::filesystem::path path, ExtensionMetaData metadata)
    : m_Path{std::move(path)}, m_MetaData{std::move(metadata)}
{}

std::vector<QObject*> IExtension::plugins() const
{
  if (!m_Loaded) {
    m_Plugins = fetchPlugins();
    m_Loaded  = true;
  }
  return m_Plugins;
}

std::vector<QObject*> IExtension::loadPlugins()
{
  // loadPlugins() is just exposed to pre-load plugins
  return plugins();
}

}  // namespace MOBase
