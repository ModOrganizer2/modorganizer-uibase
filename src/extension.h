#ifndef UIBASE_EXTENSION_H
#define UIBASE_EXTENSION_H

#include <filesystem>
#include <map>
#include <vector>

#include <QJsonObject>
#include <QTranslator>

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

class ExtensionMetaData
{
public:
  ExtensionMetaData(const IExtension* extension, QJsonObject const& jsonData);

  /**
   * @return the name of the extension.
   */
  const auto& name() const { return localized(m_Name); }

  /**
   * @return the description of the extension.
   */
  const auto& description() const { return localized(m_Description); }

  /**
   * @return the version of the extension.
   */
  const auto& version() const { return m_Version; }

  /**
   * @return the version requirement of the extension.
   */
  const auto& versionRequirement() const { return m_VersionRequirement; }

  /**
   * @return the game requirement of the extension.
   */
  const auto& gameRequirement() const { return m_GameRequirement; }

  /**
   * @return the extension requirement of the extension.
   */
  const auto& extensionRequirement() const { return m_ExtensionRequirement; }

private:
  QString localized(QString const& value) const
  {
    const auto result = QCoreApplication::translate(
        m_TranslationContext.toUtf8().data(), value.toUtf8().data());
    return result.isEmpty() ? value : result;
  }

private:
  constexpr static const char* DEFAULT_TRANSLATIONS_FOLDER = "translations";
  constexpr static const char* DEFAULT_STYLESHEET_PATH     = "stylesheets";

private:
  // these functions are only for IExtension
  //
  friend class Extension;

  // retrieve the prefix translations, if there is one, e.g., translations/foo
  //
  const auto& translationsFilePrefix() const { return m_TranslationFilesPrefix; }

  // retrieve the path to the stylesheet, if there is one
  //
  const auto& styleSheetFilePath() const { return m_StyleSheetFilePath; }

private:
  const IExtension* m_Extension;
  QString m_TranslationContext;

  QString m_Name;
  QString m_Description;
  VersionInfo m_Version;

  std::filesystem::path m_TranslationFilesPrefix;
  std::filesystem::path m_StyleSheetFilePath;

  VersionRequirement m_VersionRequirement;
  GameRequirement m_GameRequirement;
  ExtensionRequirement m_ExtensionRequirement;
};

class IExtension
{
public:
  /**
   * @brief Retrieve the plugins from this extension. If the plugins have not been
   * loaded yet, the plugins are loaded.
   *
   * @return the list of plugins from this extension.
   */
  std::vector<QObject*> plugins() const;

  /**
   * @brief Load the plugins from this extension.
   *
   * @return the list of plugins from this extension.
   */
  std::vector<QObject*> loadPlugins();

  /**
   * @brief Retrieve the path to this extension folder.
   *
   * @return the directory of this extension.
   */
  std::filesystem::path directory() const { return m_Path; }

  /**
   * @brief Retrieve the metadata of this extension.
   *
   * @return the metadata of this extension.
   */
  const auto& metadata() const { return m_MetaData; }

  /**
   * @brief Retrieve the translator of this extension for the given language.
   *
   * @param language Language to create a translator for, e.g., es, fr, fr_FR, etc.
   *
   * @return a translator for the given language, or an empty one if none exists.
   */
  QTranslator translator(QString const& language) const;

  /**
   * @brief Retrieve the stylesheet of this extension.
   */
  const QString& stylesheet() const { return m_StyleSheet; }

  virtual ~IExtension() {}

protected:
  IExtension(std::filesystem::path path, ExtensionMetaData metadata);

  // retrieve the plugins for this extension
  //
  virtual std::vector<QObject*> fetchPlugins() const = 0;

private:
  static QString createStyleSheet(std::filesystem::path const& path);
  static QString createTranslator(std::filesystem::path const& prefix,
                                  QString const& language);

private:
  std::filesystem::path m_Path;
  ExtensionMetaData m_MetaData;

  QString m_StyleSheet;

  mutable bool m_Loaded{false};
  mutable std::vector<QObject*> m_Plugins;
};

}  // namespace MOBase

#endif
