#ifndef UIBASE_EXTENSION_H
#define UIBASE_EXTENSION_H

#include <filesystem>
#include <map>
#include <vector>

#include <QJsonObject>
#include <QTranslator>

#include "dllimport.h"
#include "iplugingame.h"
#include "requirements.h"
#include "theme.h"
#include "translation.h"
#include "versioninfo.h"

namespace MOBase
{
class IExtension;

enum class ExtensionType
{
  INVALID,
  THEME,
  TRANSLATION,
  PLUGIN,
  GAME
};

class QDLLEXPORT ExtensionContributor
{
public:
  ExtensionContributor(QString name);

  // retrieve the name of the contributor
  //
  const auto& name() const { return m_Name; }

private:
  ExtensionContributor() = default;

  friend class ExtensionMetaData;

  QString m_Name;
};

class QDLLEXPORT ExtensionMetaData
{
public:
  // check if that metadata object is valid
  //
  bool isValid() const;

  // retrieve the identifier of the extension
  //
  const auto& identifier() const { return m_Identifier; }

  // retrieve the name of the extension
  //
  auto name() const { return localized(m_Name); }

  // retrieve the author of the extension if set
  //
  const auto& author() const { return m_Author; }

  // retrieve the list of contributors of the extension
  //
  const auto& contributors() const { return m_Contributors; }

  // retrieve the type of the extension
  //
  auto type() const { return m_Type; }

  // retrieve the description of the extension.
  //
  auto description() const { return localized(m_Description); }

  // retrieve the icon for the extension (might be an empty icon)
  //
  const auto& icon() const { return m_Icon; }

  // retrieve the version of the extension.
  //
  const auto& version() const { return m_Version; }

  // retrieve the raw JSON metadata, this is mostly useful for specific extension type
  // to extract custom parts
  //
  const auto& json() const { return m_JsonData; }

  // retrieve the content objects of the extension
  QJsonObject content() const;

private:
  QString localized(QString const& value) const;

private:
  friend class ExtensionFactory;

  constexpr static const char* DEFAULT_TRANSLATIONS_FOLDER = "translations";
  constexpr static const char* DEFAULT_STYLESHEET_PATH     = "stylesheets";

  ExtensionType parseType(QString const& value) const;

  ExtensionMetaData(std::filesystem::path const& path, const QJsonObject& jsonData);

private:
  QJsonObject m_JsonData;
  QString m_TranslationContext;

  QString m_Identifier;
  QString m_Name;
  ExtensionContributor m_Author;
  std::vector<ExtensionContributor> m_Contributors;
  ExtensionType m_Type;
  QString m_Description;
  QIcon m_Icon;
  VersionInfo m_Version;

  std::filesystem::path m_TranslationFilesPrefix;
  std::filesystem::path m_StyleSheetFilePath;
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

  // retrieve the requirements of the extension
  //
  const auto& requirements() const { return m_Requirements; }

  virtual ~IExtension() {}

protected:
  IExtension(std::filesystem::path path, ExtensionMetaData metadata);

private:
  std::filesystem::path m_Path;
  ExtensionMetaData m_MetaData;
  std::vector<ExtensionRequirement> m_Requirements;
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
