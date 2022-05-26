#ifndef UIBASE_EXTENSION_H
#define UIBASE_EXTENSION_H

#include <filesystem>
#include <map>
#include <vector>

#include <QJsonObject>
#include <QTranslator>

#include "dllimport.h"
#include "theme.h"
#include "translation.h"
#include "versioninfo.h"

namespace MOBase
{
class IExtension;

class VersionRequirement
{};
class GameRequirement
{};
class ExtensionRequirement
{};

enum class ExtensionType
{
  INVALID,
  THEME,
  TRANSLATION,
  PLUGIN,
  GAME
};

class QDLLEXPORT ExtensionMetaData
{
public:
  ExtensionMetaData(QJsonObject const& jsonData);

  // check if that metadata object is valid
  //
  bool isValid() const;

  // retrieve the identifier of the extension
  //
  const auto& identifier() const { return m_Identifier; }

  // retrieve the name of the extension
  //
  auto name() const { return localized(m_Name); }

  // retrieve the type of the extension
  //
  auto type() const { return m_Type; }

  // retrieve the description of the extension.
  //
  auto description() const { return localized(m_Description); }

  // retrieve the version of the extension.
  //
  const auto& version() const { return m_Version; }

  // retrieve the version requirement of the extension
  //
  const auto& versionRequirement() const { return m_VersionRequirement; }

  // retrieve the game requirement of the extension
  //
  const auto& gameRequirement() const { return m_GameRequirement; }

  // retrieve the extension requirement of the extension
  //
  const auto& extensionRequirement() const { return m_ExtensionRequirement; }

  // retrieve the raw JSON metadata, this is mostly useful for specific extension type
  // to extract custom parts
  //
  const auto& json() const { return m_JsonData; }

private:
  QString localized(QString const& value) const;

private:
  constexpr static const char* DEFAULT_TRANSLATIONS_FOLDER = "translations";
  constexpr static const char* DEFAULT_STYLESHEET_PATH     = "stylesheets";

  ExtensionType parseType(QString const& value) const;

private:
  QJsonObject m_JsonData;
  QString m_TranslationContext;

  QString m_Identifier;
  QString m_Name;
  ExtensionType m_Type;
  QString m_Description;
  VersionInfo m_Version;

  std::filesystem::path m_TranslationFilesPrefix;
  std::filesystem::path m_StyleSheetFilePath;

  VersionRequirement m_VersionRequirement;
  GameRequirement m_GameRequirement;
  ExtensionRequirement m_ExtensionRequirement;
};

class QDLLEXPORT IExtension
{
public:
  // retrieve the folder containing the extension
  //
  std::filesystem::path directory() const { return m_Path; }

  // retrieve the metadata of this extension
  //
  const auto& metadata() const { return m_MetaData; }

  virtual ~IExtension() {}

protected:
  IExtension(std::filesystem::path path, ExtensionMetaData metadata);

private:
  std::filesystem::path m_Path;
  ExtensionMetaData m_MetaData;
};

// factory for extensions
//
class QDLLEXPORT ExtensionFactory
{
public:
  // load an extension from the given directory, return a null-pointer if the extension
  // could not be load
  //
  static std::unique_ptr<IExtension> loadExtension(std::filesystem::path directory);

private:
  // name of the metadata file
  //
  static constexpr const char* METADATA_FILENAME = "mo2-metadata.json";

  // load an extension from the given directory
  //
  static std::unique_ptr<IExtension> loadExtension(std::filesystem::path directory,
                                                   ExtensionMetaData metadata);
};

// theme extension that provides one or more base themes for MO2
//
class QDLLEXPORT ThemeExtension : public IExtension
{
public:
  // retrieve the list of themes provided by this extension
  //
  const auto& themes() const { return m_Themes; }

private:
  ThemeExtension(std::filesystem::path path, ExtensionMetaData metadata,
                 std::vector<std::shared_ptr<const Theme>> themes);

  friend class ExtensionFactory;
  static std::unique_ptr<ThemeExtension> loadExtension(std::filesystem::path path,
                                                       ExtensionMetaData metadata);

  static std::shared_ptr<const Theme>
  parseTheme(std::filesystem::path const& extensionFolder, const QString& identifier,
             const QJsonObject& jsonTheme);

private:
  std::vector<std::shared_ptr<const Theme>> m_Themes;
};

// translation extension that provides one or more base translations for mo@
//
class QDLLEXPORT TranslationExtension : public IExtension
{
public:
  // retrieve the list of translations provided by this extension
  //
  const auto& translations() const { return m_Translations; }

private:
  TranslationExtension(std::filesystem::path path, ExtensionMetaData metadata,
                       std::vector<std::shared_ptr<const Translation>> translations);

  friend class ExtensionFactory;
  static std::unique_ptr<TranslationExtension>
  loadExtension(std::filesystem::path path, ExtensionMetaData metadata);

  static std::shared_ptr<const Translation>
  parseTranslation(std::filesystem::path const& extensionFolder,
                   const QString& identifier, const QJsonObject& jsonTranslation);

private:
  std::vector<std::shared_ptr<const Translation>> m_Translations;
};

// plugin extension that provides one or more plugins for MO2, alongside theme or
// translation additions
//
class QDLLEXPORT PluginExtension : public IExtension
{
public:
  using IExtension::IExtension;

  // auto-detect plugins
  //
  bool autodetect() const { return m_AutoDetect; }

  // list of specified plugins
  //
  const auto& plugins() const { return m_Plugins; }

  const auto& themeAdditions() const { return m_ThemeAdditions; }
  const auto& translationAdditions() const { return m_TranslationAdditions; }

protected:
  PluginExtension(
      std::filesystem::path path, ExtensionMetaData metadata, bool autodetect,
      std::map<std::string, std::filesystem::path> plugins,
      std::vector<std::shared_ptr<const ThemeAddition>> themeAdditions,
      std::vector<std::shared_ptr<const TranslationAddition>> translationAdditions);

  friend class ExtensionFactory;
  static std::unique_ptr<PluginExtension> loadExtension(std::filesystem::path path,
                                                        ExtensionMetaData metadata);

private:
  // auto-detect plugins
  bool m_AutoDetect;

  // forced plugins
  std::map<std::string, std::filesystem::path> m_Plugins;

  // theme and translations additions
  std::vector<std::shared_ptr<const ThemeAddition>> m_ThemeAdditions;
  std::vector<std::shared_ptr<const TranslationAddition>> m_TranslationAdditions;
};

// game extension that provides a game plugin, alongside other plugins, translation or
// theme (additions)
//
class QDLLEXPORT GameExtension : public PluginExtension
{
private:
  GameExtension(PluginExtension&& pluginExtension);

  friend class ExtensionFactory;
  static std::unique_ptr<GameExtension> loadExtension(std::filesystem::path path,
                                                      ExtensionMetaData metadata);
};

}  // namespace MOBase

#endif
