#include "extension.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

#include "log.h"

namespace MOBase
{

namespace
{
  // retrieve all files matching one of the glob pattern in globPatterns, assuming paths
  // (in patterns) are relative to basePath
  //
  auto globExtensionFiles(std::filesystem::path basePath,
                          QStringList const& globPatterns)
  {
    std::vector<std::filesystem::path> files;

    for (const auto& globFile : globPatterns) {
      // use a QFileInfo to extract the name (glob) and the directory - we currently do
      // not handle recursive glob (**)
      QFileInfo globFileInfo(basePath, globFile);

      QDirIterator dirIterator(globFileInfo.absolutePath(), {globFileInfo.fileName()},
                               QDir::Files);
      while (dirIterator.hasNext()) {
        dirIterator.next();
        files.push_back(dirIterator.fileInfo().filesystemAbsoluteFilePath());
      }
    }

    return files;
  }

}  // namespace

ExtensionMetaData::ExtensionMetaData(QJsonObject const& jsonData) : m_JsonData{jsonData}
{
  // read basic fields
  m_Identifier  = jsonData["identifier"].toString();
  m_Type        = parseType(jsonData["type"].toString());
  m_Name        = jsonData["name"].toString();
  m_Description = jsonData["description"].toString();
  m_Version.parse(jsonData["version"].toString("0.0.0"));

  // TODO: name of the key
  // translation context
  m_TranslationContext = jsonData["translationContext"].toString("");
}

bool ExtensionMetaData::isValid() const
{
  return !m_Identifier.isEmpty() && !m_Name.isEmpty() && m_Version.isValid() &&
         m_Type != ExtensionType::INVALID;
}

ExtensionType ExtensionMetaData::parseType(QString const& value) const
{
  std::map<QString, ExtensionType> stringToTypes{
      {"theme", ExtensionType::THEME},
      {"translation", ExtensionType::TRANSLATION},
      {"plugin", ExtensionType::PLUGIN},
      {"game", ExtensionType::GAME}};

  auto type = ExtensionType::INVALID;
  for (auto& [k, v] : stringToTypes) {
    if (k.compare(value, Qt::CaseInsensitive) == 0) {
      type = v;
      break;
    }
  }

  return type;
}

QString ExtensionMetaData::localized(QString const& value) const
{
  // no translation context
  if (m_TranslationContext.isEmpty()) {
    return value;
  }

  const auto result = QCoreApplication::translate(m_TranslationContext.toUtf8().data(),
                                                  value.toUtf8().data());
  return result.isEmpty() ? value : result;
}

IExtension::IExtension(std::filesystem::path path, ExtensionMetaData metadata)
    : m_Path{std::move(path)}, m_MetaData{std::move(metadata)}
{}

std::unique_ptr<IExtension>
ExtensionFactory::loadExtension(std::filesystem::path directory)
{
  const auto metadataPath = directory / METADATA_FILENAME;

  if (!exists(metadataPath)) {
    log::warn("missing extension metadata in '{}'", directory.native());
    return nullptr;
  }

  // load the meta data
  QJsonParseError jsonError;
  QJsonDocument jsonMetaData;
  {
    QFile file(metadataPath);
    if (!file.open(QFile::ReadOnly)) {
      return {};
    }

    const auto jsonContent = file.readAll();
    jsonMetaData           = QJsonDocument::fromJson(jsonContent, &jsonError);
  }

  if (jsonMetaData.isNull()) {
    log::warn("failed to read metadata from '{}': {}", metadataPath.native(),
              jsonError.errorString());
    return nullptr;
  }

  return loadExtension(std::move(directory), ExtensionMetaData(jsonMetaData.object()));
}

std::unique_ptr<IExtension>
ExtensionFactory::loadExtension(std::filesystem::path directory,
                                ExtensionMetaData metadata)
{
  if (!metadata.isValid()) {
    log::warn("failed to load extension from '{}': invalid metadata",
              directory.native());
    return nullptr;
  }

  switch (metadata.type()) {
  case ExtensionType::THEME:
    return ThemeExtension::loadExtension(std::move(directory), std::move(metadata));
  case ExtensionType::TRANSLATION:
    return TranslationExtension::loadExtension(std::move(directory),
                                               std::move(metadata));
  case ExtensionType::PLUGIN:
    return PluginExtension::loadExtension(std::move(directory), std::move(metadata));
  case ExtensionType::GAME:
    return GameExtension::loadExtension(std::move(directory), std::move(metadata));
  case ExtensionType::INVALID:
  default:
    log::warn("failed to load extension from '{}': invalid type", directory.native());
    return nullptr;
  }
}

ThemeExtension::ThemeExtension(std::filesystem::path path, ExtensionMetaData metadata,
                               std::vector<std::shared_ptr<const Theme>> themes)
    : IExtension{std::move(path), std::move(metadata)}, m_Themes{std::move(themes)}
{}

std::unique_ptr<ThemeExtension>
ThemeExtension::loadExtension(std::filesystem::path path, ExtensionMetaData metadata)
{
  std::vector<std::shared_ptr<const Theme>> themes;
  const auto& jsonThemes = metadata.json()["themes"].toObject();
  for (auto it = jsonThemes.begin(); it != jsonThemes.end(); ++it) {
    const auto theme = parseTheme(path, it.key(), it.value().toObject());
    if (theme) {
      themes.push_back(theme);
    } else {
      log::warn("failed to parse theme '{}' from '{}'", it.key(), path.native());
    }
  }

  if (themes.empty()) {
    log::error("failed to parse themes from '{}'", path.native());
    return nullptr;
  }

  return std::unique_ptr<ThemeExtension>{
      new ThemeExtension(path, metadata, std::move(themes))};
}

std::shared_ptr<const Theme>
ThemeExtension::parseTheme(std::filesystem::path const& extensionFolder,
                           const QString& identifier, const QJsonObject& jsonTheme)
{
  const auto name = jsonTheme["name"].toString();
  const auto filepath =
      extensionFolder / jsonTheme["path"].toString().toUtf8().toStdString();

  if (name.isEmpty() || !is_regular_file(filepath)) {
    return nullptr;
  }

  return std::make_shared<Theme>(identifier.toStdString(), name.toStdString(),
                                 filepath);
}

TranslationExtension::TranslationExtension(
    std::filesystem::path path, ExtensionMetaData metadata,
    std::vector<std::shared_ptr<const Translation>> translations)
    : IExtension{std::move(path), std::move(metadata)},
      m_Translations(std::move(translations))
{}

std::unique_ptr<TranslationExtension>
TranslationExtension::loadExtension(std::filesystem::path path,
                                    ExtensionMetaData metadata)
{
  std::vector<std::shared_ptr<const Translation>> translations;
  const auto& jsonTranslations = metadata.json()["translations"].toObject();
  for (auto it = jsonTranslations.begin(); it != jsonTranslations.end(); ++it) {
    const auto translation = parseTranslation(path, it.key(), it.value().toObject());
    if (translation) {
      translations.push_back(translation);
    } else {
      log::warn("failed to parse translation '{}' from '{}'", it.key(), path.native());
    }
  }

  if (translations.empty()) {
    log::error("failed to parse translations from '{}'", path.native());
    return nullptr;
  }

  return std::unique_ptr<TranslationExtension>{
      new TranslationExtension(path, metadata, std::move(translations))};
}

std::shared_ptr<const Translation>
TranslationExtension::parseTranslation(std::filesystem::path const& extensionFolder,
                                       const QString& identifier,
                                       const QJsonObject& jsonTranslation)
{
  const auto name          = jsonTranslation["name"].toString();
  const auto jsonGlobFiles = jsonTranslation["files"].toVariant().toStringList();

  std::vector<std::filesystem::path> qm_files =
      globExtensionFiles(extensionFolder, jsonGlobFiles);

  if (name.isEmpty() || qm_files.empty()) {
    return nullptr;
  }

  return std::make_shared<Translation>(identifier.toStdString(), name.toStdString(),
                                       std::move(qm_files));
}

PluginExtension::PluginExtension(
    std::filesystem::path path, ExtensionMetaData metadata, bool autodetect,
    std::vector<std::filesystem::path> plugins,
    std::vector<std::shared_ptr<const ThemeAddition>> themeAdditions,
    std::vector<std::shared_ptr<const TranslationAddition>> translationAdditions)
    : IExtension(std::move(path), std::move(metadata)), m_AutoDetect{autodetect},
      m_Plugins{std::move(plugins)}, m_ThemeAdditions{std::move(themeAdditions)},
      m_TranslationAdditions{std::move(translationAdditions)}
{}

std::unique_ptr<PluginExtension>
PluginExtension::loadExtension(std::filesystem::path path, ExtensionMetaData metadata)
{
  // load themes
  std::vector<std::shared_ptr<const ThemeAddition>> themes;
  {
    const auto& jsonThemes = metadata.json()["themes"].toObject();
    for (auto it = jsonThemes.begin(); it != jsonThemes.end(); ++it) {
      for (auto& file : globExtensionFiles(path, it.value().toVariant().toStringList()))
        themes.push_back(std::make_shared<ThemeAddition>(it.key().toStdString(), file));
    }
  }

  // load translations
  std::vector<std::shared_ptr<const TranslationAddition>> translations;
  {
    const auto& jsonTranslations = metadata.json()["translations"].toObject();

    // * is a custom entry
    if (jsonTranslations.contains("*")) {
      std::map<QString, std::vector<std::filesystem::path>> filesPerLanguage;
      const auto prefixes = jsonTranslations["*"].toVariant().toStringList();

      for (auto& prefix : prefixes) {
        const auto filePrefix = QFileInfo(prefix).fileName();
        auto files            = globExtensionFiles(path, {prefix + "*.qm"});
        for (auto& file : files) {
          // extract the identifier from the match, e.g., if the prefix is
          // translations/installer_manual_, the filePrefix is installer_manual_,
          // and glob will be like translations/installer_manual_fr.qm
          const auto identifier =
              QFileInfo(file).fileName().replace(".qm", "").replace(filePrefix, "");
          filesPerLanguage[identifier].push_back(file);
        }
      }

      for (auto& [language, files] : filesPerLanguage) {
        translations.push_back(std::make_shared<TranslationAddition>(
            language.toStdString(), std::move(files)));
      }

    } else {
      for (auto it = jsonTranslations.begin(); it != jsonTranslations.end(); ++it) {
        translations.push_back(std::make_shared<TranslationAddition>(
            it.key().toStdString(),
            globExtensionFiles(path, it.value().toVariant().toStringList())));
      }
    }
  }

  return std::unique_ptr<PluginExtension>(
      new PluginExtension(std::move(path), std::move(metadata), true, {},
                          std::move(themes), std::move(translations)));
}

GameExtension::GameExtension(PluginExtension&& pluginExtension)
    : PluginExtension(std::move(pluginExtension))
{}

std::unique_ptr<GameExtension> GameExtension::loadExtension(std::filesystem::path path,
                                                            ExtensionMetaData metadata)
{
  auto extension = PluginExtension::loadExtension(std::move(path), std::move(metadata));
  return extension
             ? std::unique_ptr<GameExtension>(new GameExtension(std::move(*extension)))
             : nullptr;
}

}  // namespace MOBase
