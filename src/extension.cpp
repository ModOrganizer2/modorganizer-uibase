#include "extension.h"

#include <QFile>
#include <QFileInfo>

namespace MOBase
{

QString IExtension::createStyleSheet(std::filesystem::path const& path)
{
  if (!exists(path)) {
    return "";
  }

  QString stylesheet;

  // read the whole file
  {
    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      return "";
    }

    stylesheet = QTextStream(&file).readAll();
  }

  // replace url() in the file
  // TODO

  return stylesheet;
}

QTranslator IExtension::createTranslator(std::filesystem::path const& prefix,
                                         QString const& language)
{
  const QFileInfo fileInfo(prefix);

  QTranslator translator;

  if (fileInfo.exists()) {
    translator.load(fileInfo.fileName() + "_" + language, fileInfo.absolutePath());
  }

  return translator
}

IExtension::IExtension(std::filesystem::path path, ExtensionMetaData metadata)
    : m_Path{std::move(path)}, m_MetaData{std::move(metadata)}
{
  m_StyleSheet = createStyleSheet(m_MetaData.styleSheetFilePath());
}

QTranslator IExtension::translator(QString const& language) const
{
  return createTranslator(m_MetaData.translationsFilePrefix(), language);
}

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
