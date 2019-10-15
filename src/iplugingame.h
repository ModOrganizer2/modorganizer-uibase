#ifndef IPLUGINGAME_H
#define IPLUGINGAME_H


#include "iplugin.h"
#include "executableinfo.h"

class QIcon;
class QUrl;
class QStringList;

#include <boost/any.hpp>

#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace MOBase {


class IPluginGame : public QObject, public IPlugin {
  Q_INTERFACES(IPlugin)

public:

  enum class LoadOrderMechanism {
    FileTime,
    PluginsTxt
  };

  enum class SortMechanism {
    NONE,
    MLOX,
    BOSS,
    LOOT
  };

  enum ProfileSetting {
    MODS            = 0x01,
    CONFIGURATION   = 0x02,
    SAVEGAMES       = 0x04,
    PREFER_DEFAULTS = 0x08
  };
  Q_DECLARE_FLAGS(ProfileSettings, ProfileSetting)

public:

  /**
   * @return name of the game
   */
  virtual QString gameName() const = 0;

  template <typename T>
  T *feature() const {
    auto list = featureList();
    auto iter = list.find(typeid(T));
    if (iter != list.end()) {
      try {
        return boost::any_cast<T*>(iter->second);
      } catch (const boost::bad_any_cast&) {
        // don't use log::error() here so log.h and fmt aren't pulled into
        // plugins
        qCritical(
          "failed to retrieve feature type %s (got %s)",
          typeid(T).name(), typeid(iter->second).name());
        return nullptr;
      }
    } else {
      return nullptr;
    }
  }

  /**
   * @brief initialize a profile for this game
   * @param directory the directory where the profile is to be initialized
   * @param settings parameters for how the profile should be initialized
   * @note the MO app does not yet support virtualizing only specific aspects but plugins should be written with this future functionality in mind
   * @note this function will be used to initially create a profile, potentially to repair it or upgrade/downgrade it so the implementations
   *       have to gracefully handle the case that the directory already contains files!
   */
  virtual void initializeProfile(const QDir &directory, ProfileSettings settings) const = 0;

  /**
   * @return file extension of save games for this game
   */
  virtual QString savegameExtension() const = 0;

  /**
   * @return file extension of script extender save game files for this game
   */
  virtual QString savegameSEExtension() const = 0;

  /**
   * @return true if this game has been discovered as installed, false otherwise
   */
  virtual bool isInstalled() const = 0;

  /**
   * @return an icon for this game
   */
  virtual QIcon gameIcon() const = 0;

  /**
   * @return directory to the game installation
   */
  virtual QDir gameDirectory() const = 0;

  /**
   * @return directory where the game expects to find its data files
   */
  virtual QDir dataDirectory() const = 0;

  /**
   * @brief set the path to the managed game
   * @param path to the game
   * @note this will be called by by MO to set the concrete path of the game. This is particularly
   *       relevant if the path wasn't auto-detected but had to be set manually by the user
   */
  virtual void setGamePath(const QString &path) = 0;

  /**
   * @return directory of the documents folder where configuration files and such for this game reside
   */
  virtual QDir documentsDirectory() const = 0;

  /**
   * @return path to where save games are stored.
   */
  virtual QDir savesDirectory() const = 0;

  /**
   * @return list of automatically discovered executables of the game itself and tools surrounding it
   */
  virtual QList<ExecutableInfo> executables() const = 0;

  /**
   * @brief Get the default list of libraries that can be force loaded with executables
   */
  virtual QList<ExecutableForcedLoadSetting> executableForcedLoads() const = 0;

  /**
   * @return steam app id for this game. Should be empty for games not available on steam
   * @note if a game is available in multiple versions those might have different app ids.
   *       the plugin should try to return the right one
   */
  virtual QString steamAPPId() const = 0;

  /**
   * @return list of plugins that are part of the game and not considered optional
   */
  virtual QStringList primaryPlugins() const = 0;

  /**
   * @return list of game variants
   * @note if there are multiple variants of a game (and the variants make a difference to the
   *       plugin) like a regular one and a GOTY-edition the plugin can return a list of them and
   *       the user gets to chose which one he owns.
   */
  virtual QStringList gameVariants() const = 0;

  /**
   * @brief if there are multiple game variants (returned by gameVariants) this will get called
   *        on start with the user-selected game edition
   * @param variant the game edition selected by the user
   */
  virtual void setGameVariant(const QString &variant) = 0;

  /**
   * @brief Get the name of the executable that gets run
   */
  virtual QString binaryName() const = 0;

  /**
   * @brief Get the 'short' name of the game
   *
   * the short name of the game is used for - save ames, registry entries and
   * nexus mod pages as far as I can see.
   */
  virtual QString gameShortName() const = 0;

  /**
   * @brief Get any primary alternative 'short' name for the game
   *
   * this is used to determine if a Nexus (or other) download source should be considered
   * a 'primary' source for the game so that it isn't flagged as an alternative source
   */
  virtual QStringList primarySources() const = 0;

  /**
   * @brief Get any valid 'short' name for the game
   *
   * this is used to determine if a Nexus download is valid for the current game
   * not all game variants have their own nexus pages and others can handle downloads
   * from other nexus game pages and should be allowed
   *
   * the short name should be considered the primary handler for a directly supported game
   * for puroses of auto-launching an instance
   */
  virtual QStringList validShortNames() const = 0;

   /**
   * @brief Get the 'short' name of the game
   *
   * the short name of the game is used for - save ames, registry entries and
   * nexus mod pages as far as I can see.
   */
  virtual QString gameNexusName() const = 0;

  /**
   * @brief Get the list of .ini files this game uses
   *
   * @note It is important that the 'main' .ini file comes first in this list
   */
  virtual QStringList iniFiles() const = 0;

  /**
   * @brief Get a list of esp/esm files that are part of known dlcs
   */
  virtual QStringList DLCPlugins() const = 0;

  /**
   * @brief Get the current list of active Creation Club plugins
   */
  virtual QStringList CCPlugins() const = 0;

  /*
   * @brief determine the load order mechanism used by this game.
   *
   * @note this may throw an exception if the mechanism can't be determined
   */
  virtual LoadOrderMechanism loadOrderMechanism() const = 0;

  /**
   * @brief determine the sorting mech
   */
  virtual SortMechanism sortMechanism() const = 0;

  /**
   * @brief Get the Nexus ID of Mod Organizer
   */
  virtual int nexusModOrganizerID() const = 0;

  /**
   * @brief Get the Nexus Game ID
   */
  virtual int nexusGameID() const = 0;

  /**
   * @brief See if the supplied directory looks like a valid game
   */
  virtual bool looksValid(QDir const &) const = 0;

  /**
   * @brief Get version of program
   */
  virtual QString gameVersion() const = 0;

  /**
   * @brief Get the name of the game launcher
   */
  virtual QString getLauncherName() const = 0;

  /**
   * @brief Get whether the game supports light plugins (ESL,.esl)
   */
  virtual bool supportsLightPlugins() const = 0;

protected:

  virtual std::map<std::type_index, boost::any> featureList() const = 0;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(IPluginGame::ProfileSettings)

} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginGame, "com.tannin.ModOrganizer.PluginGame/2.0")
Q_DECLARE_METATYPE(MOBase::IPluginGame const *)

#endif // IPLUGINGAME_H
