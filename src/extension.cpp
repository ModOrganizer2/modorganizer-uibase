#include "extensions/extension.h"

#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>

#include "log.h"

// name of the metadata file
//
static constexpr const char* METADATA_FILENAME = "metadata.json";

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

  // parse an author from a JSON value
  //
  ExtensionContributor parseContributor(QJsonValue const& value)
  {
    if (value.isNull()) {
      return ExtensionContributor("");
    }

    // TODO: handle more fields in the future, handle string authors similar to NPM

    if (value.isObject()) {
      const auto contrib = value.toObject();
      return ExtensionContributor(contrib["name"].toString());
    }

    return ExtensionContributor(value.toString());
  }

}  // namespace

ExtensionContributor::ExtensionContributor(QString name) : m_Name{name} {}

ExtensionMetaData::ExtensionMetaData(std::filesystem::path const& path,
                                     QJsonObject const& jsonData)
    : m_JsonData{jsonData}, m_Version{0, 0, 0}, m_Requirements{}
{
  // read basic fields
  m_Identifier = jsonData["id"].toString();
  if (m_Identifier.isEmpty()) {
    throw InvalidExtensionMetaDataException("missing identifier");
  }

  {
    const auto maybeType = parseType(jsonData["type"].toString());
    if (!maybeType.has_value()) {
      throw InvalidExtensionMetaDataException(
          std::format("invalid or missing type '{}'", jsonData["type"].toString()));
    }

    m_Type = *maybeType;
  }

  m_Name = jsonData["name"].toString();
  if (m_Name.isEmpty()) {
    throw InvalidExtensionMetaDataException("missing name");
  }

  m_Author      = parseContributor(jsonData["author"]);
  m_Description = jsonData["description"].toString();

  try {
    m_Version = Version::parse(jsonData["version"].toString("0.0.0"),
                               Version::ParseMode::SemVer);
  } catch (InvalidVersionException const& ex) {
    throw InvalidExtensionMetaDataException(
        std::format("invalid or missing version '{}'", jsonData["version"].toString()));
  }

  // TODO: name of the key
  // translation context
  m_TranslationContext = jsonData["translation-context"].toString("");

  if (jsonData.contains("icon")) {
    const QFileInfo icon{QDir(path), jsonData["icon"].toString()};
    if (icon.exists()) {
      m_Icon = QIcon(icon.absoluteFilePath());
    }
  }

  if (jsonData.contains("contributors")) {
    for (const auto& jsonContributor : jsonData["contributors"].toArray()) {
      m_Contributors.push_back(parseContributor(jsonContributor));
    }
  }

  if (jsonData.contains("requirements")) {
    try {
      m_Requirements =
          ExtensionRequirementFactory::parseRequirements(jsonData["requirements"]);
    } catch (InvalidRequirementException const& ex) {
      throw InvalidExtensionMetaDataException(ex.what());
    } catch (InvalidRequirementsException const& ex) {
      throw InvalidExtensionMetaDataException(ex.what());
    }
  }
}

std::optional<ExtensionType> ExtensionMetaData::parseType(QString const& value) const
{
  std::map<QString, ExtensionType> stringToTypes{
      {"theme", ExtensionType::THEME},
      {"translation", ExtensionType::TRANSLATION},
      {"plugin", ExtensionType::PLUGIN},
      {"game", ExtensionType::GAME}};

  std::optional<ExtensionType> type;
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

QJsonObject ExtensionMetaData::content() const
{
  if (!m_JsonData.contains("content")) {
    return {};
  }

  const auto value = m_JsonData["content"];
  if (!value.isObject()) {
    log::error("invalid metadata for {}, 'content' should be an object", m_Identifier);
    return {};
  }

  return value.toObject();
}

IExtension::IExtension(std::filesystem::path const& path, ExtensionMetaData&& metadata)
    : m_Path{path}, m_MetaData{std::move(metadata)}
{}

ExtensionMetaData ExtensionFactory::loadMetaData(std::filesystem::path const& path)
{
  if (!exists(path)) {
    throw InvalidExtensionMetaDataException(
        std::format("metadata file '{}' not found", path));
  }

  // load the meta data
  QJsonParseError jsonError;
  QJsonDocument jsonMetaData;
  {
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
      throw InvalidExtensionMetaDataException(
          std::format("failed to open metadata file '{}'", path));
    }

    const auto jsonContent = file.readAll();
    jsonMetaData           = QJsonDocument::fromJson(jsonContent, &jsonError);
  }

  if (jsonMetaData.isNull()) {
    throw InvalidExtensionMetaDataException(
        std::format("invalid metadata file '{}': {}", path, jsonError.errorString()));
  }

  return ExtensionMetaData(path.parent_path(), jsonMetaData.object());
}

std::unique_ptr<IExtension>
ExtensionFactory::loadExtension(std::filesystem::path const& directory)
{
  try {
    return loadExtension(directory, loadMetaData(directory / METADATA_FILENAME));
  } catch (InvalidExtensionMetaDataException const& ex) {
    log::warn("failed to load extension from '{}': invalid metadata ({})",
              directory.native(), ex.what());
    return nullptr;
  }
}

std::unique_ptr<IExtension>
ExtensionFactory::loadExtension(std::filesystem::path const& directory,
                                ExtensionMetaData&& metadata)
{
  switch (metadata.type()) {
  case ExtensionType::THEME:
    return ThemeExtension::loadExtension(directory, std::move(metadata));
  case ExtensionType::TRANSLATION:
    return TranslationExtension::loadExtension(directory, std::move(metadata));
  case ExtensionType::PLUGIN:
    return PluginExtension::loadExtension(directory, std::move(metadata));
  case ExtensionType::GAME:
    return GameExtension::loadExtension(directory, std::move(metadata));
  default:
    log::warn("failed to load extension from '{}': invalid type", directory.native());
    return nullptr;
  }
}

ThemeExtension::ThemeExtension(std::filesystem::path const& path,
                               ExtensionMetaData&& metadata,
                               std::vector<std::shared_ptr<const Theme>> themes)
    : IExtension{path, std::move(metadata)}, m_Themes{std::move(themes)}
{}

std::unique_ptr<ThemeExtension>
ThemeExtension::loadExtension(std::filesystem::path const& path,
                              ExtensionMetaData&& metadata)
{
  std::vector<std::shared_ptr<const Theme>> themes;
  const auto& jsonThemes = metadata.content()["themes"].toObject();
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
      new ThemeExtension(path, std::move(metadata), std::move(themes))};
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
    std::filesystem::path const& path, ExtensionMetaData&& metadata,
    std::vector<std::shared_ptr<const Translation>> translations)
    : IExtension{std::move(path), std::move(metadata)},
      m_Translations(std::move(translations))
{}

std::unique_ptr<TranslationExtension>
TranslationExtension::loadExtension(std::filesystem::path const& path,
                                    ExtensionMetaData&& metadata)
{
  std::vector<std::shared_ptr<const Translation>> translations;
  const auto& jsonTranslations = metadata.content()["translations"].toObject();
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
      new TranslationExtension(path, std::move(metadata), std::move(translations))};
}

std::shared_ptr<const Translation>
TranslationExtension::parseTranslation(std::filesystem::path const& extensionFolder,
                                       const QString& identifier,
                                       const QJsonObject& jsonTranslation)
{
  const auto jsonGlobFiles = jsonTranslation["files"].toVariant().toStringList();

  std::vector<std::filesystem::path> qm_files =
      globExtensionFiles(extensionFolder, jsonGlobFiles);

  if (qm_files.empty()) {
    return nullptr;
  }

  const auto jsonName = jsonTranslation["name"];
  QString name;
  if (jsonName.isString()) {
    name = jsonName.toString();
  } else {
    QLocale locale(identifier);
    name = QString("%1 (%2)")
               .arg(locale.nativeLanguageName())
               .arg(locale.nativeCountryName());
  }

  return std::make_shared<Translation>(identifier.toStdString(), name.toStdString(),
                                       std::move(qm_files));
}

PluginExtension::PluginExtension(
    std::filesystem::path const& path, ExtensionMetaData&& metadata, bool autodetect,
    std::map<std::string, std::filesystem::path> plugins,
    std::vector<std::shared_ptr<const ThemeAddition>> themeAdditions,
    std::vector<std::shared_ptr<const TranslationAddition>> translationAdditions)
    : IExtension(path, std::move(metadata)), m_AutoDetect{autodetect},
      m_Plugins{std::move(plugins)}, m_ThemeAdditions{std::move(themeAdditions)},
      m_TranslationAdditions{std::move(translationAdditions)}
{}

std::unique_ptr<PluginExtension>
PluginExtension::loadExtension(std::filesystem::path const& path,
                               ExtensionMetaData&& metadata)
{
  namespace fs = std::filesystem;

  // load plugins
  std::optional<bool> autodetect;
  std::map<std::string, fs::path> plugins;
  {
    auto jsonPlugins = metadata.content()["plugins"].toObject();
    if (jsonPlugins.contains("autodetect")) {
      autodetect = jsonPlugins["autodetect"].toBool();
    }
    jsonPlugins.remove("autodetect");

    for (auto it = jsonPlugins.begin(); it != jsonPlugins.end(); ++it) {
      plugins[it.key().toStdString()] =
          QFileInfo(path, it.value().toString()).filesystemAbsoluteFilePath();
    }

    if (!autodetect.has_value()) {
      autodetect = plugins.empty();
    }
  }

  // load themes
  std::vector<std::shared_ptr<const ThemeAddition>> themes;
  {
    auto jsonThemes = metadata.json()["themes"].toObject();
    for (auto it = jsonThemes.begin(); it != jsonThemes.end(); ++it) {
      for (auto& file : globExtensionFiles(path, it.value().toVariant().toStringList()))
        themes.push_back(std::make_shared<ThemeAddition>(it.key().toStdString(), file));
    }
  }

  // load translations
  std::vector<std::shared_ptr<const TranslationAddition>> translations;
  {
    auto jsonTranslations = metadata.json()["translations"].toObject();

    if (jsonTranslations.contains("*")) {
      // * is a custom entry - * should point to a list of file prefix, e.g.,
      // ["translations/foo_", "translations/bar_"] meaning that the translations
      // files are prefixed by foo_ and bar_ inside the translations folder, language
      // is extracted by removing the prefix
      //
      // TODO: remove this option
      //
      std::map<QString, std::vector<fs::path>> filesPerLanguage;
      const auto prefixes = jsonTranslations["*"].toVariant().toStringList();

      for (auto& prefix : prefixes) {
        const auto filePrefix = QFileInfo(prefix).fileName();
        const auto files      = globExtensionFiles(path, {prefix + "*.qm"});
        for (auto& file : files) {
          // extract the identifier from the match, e.g., if the prefix is
          // translations/installer_manual_, the filePrefix is installer_manual_,
          // and glob will be like translations/installer_manual_fr.qm
          const auto identifier =
              QFileInfo(file).baseName().replace(filePrefix, "", Qt::CaseInsensitive);
          filesPerLanguage[identifier].push_back(file);
        }
      }

      for (auto& [language, files] : filesPerLanguage) {
        translations.push_back(std::make_shared<TranslationAddition>(
            language.toStdString(), std::move(files)));
      }
    } else if (jsonTranslations.contains("autodetect")) {

      // autodetect is a custom entry - "autodetect": "xxx" means that the extension
      // contains a translations folder named "xxx" where each subfolder is a language
      // (identifier) containing translation files for the language

      std::map<QString, std::vector<std::filesystem::path>> filesPerLanguage;
      const auto folder = jsonTranslations["autodetect"].toString();
      for (const auto& lang : fs::directory_iterator(path / folder.toStdString())) {
        if (!fs::is_directory(lang)) {
          continue;
        }

        filesPerLanguage[QString::fromStdWString(lang.path().filename().wstring())] =
            globExtensionFiles(lang, {"*.qm"});
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

  return std::unique_ptr<PluginExtension>(new PluginExtension(
      std::move(path), std::move(metadata), *autodetect, std::move(plugins),
      std::move(themes), std::move(translations)));
}

GameExtension::GameExtension(PluginExtension&& pluginExtension)
    : PluginExtension(std::move(pluginExtension))
{}

std::unique_ptr<GameExtension>
GameExtension::loadExtension(std::filesystem::path const& path,
                             ExtensionMetaData&& metadata)
{
  auto extension = PluginExtension::loadExtension(std::move(path), std::move(metadata));
  return extension
             ? std::unique_ptr<GameExtension>(new GameExtension(std::move(*extension)))
             : nullptr;
}

}  // namespace MOBase
