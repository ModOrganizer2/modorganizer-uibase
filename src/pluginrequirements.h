#ifndef PLUGINREQUIREMENTS_H
#define PLUGINREQUIREMENTS_H

#include <functional>

#include <QStringList>

#include "dllimport.h"

namespace MOBase {

class IOrganizer;
class IPluginDiagnose;

/**
 * @brief The interface for plugin requirements.
 */
class IPluginRequirement {
public:

  /**
   * @brief Check if the requirements is met.
   *
   * @param organizer The organizer proxy.
   *
   * @return the list of problem IDs.
   */
  virtual std::vector<unsigned int> problems(IOrganizer* organizer) const = 0;

  /**
   * @brief Return a for the given problem.
   *
   * @param id The ID of the problem.
   *
   * @return a message indicating why the requirement was not met.
   */
  virtual QString description(unsigned int id) const = 0;

  virtual ~IPluginRequirement() { }
};

/**
 * @brief Plugin dependency - The requirement is met if one of the
 *     given plugin is active.
 */
class PluginDependencyRequirement : public IPluginRequirement {

  friend class PluginRequirementFactory;

public:

  std::vector<unsigned int> problems(IOrganizer*) const override;
  QString description(unsigned int) const override;

  /**
   * @return the list of plugin names in this dependency requirement.
   */
  QStringList pluginNames() const { return m_PluginNames; }

protected:
  PluginDependencyRequirement(QStringList const& pluginNames);

  QStringList m_PluginNames;
};

/**
 * @brief Game dependency - The requirement is met if the active game
 *     is one of the specified game.
 */
class GameDependencyRequirement : public IPluginRequirement {

  friend class PluginRequirementFactory;

public:

  std::vector<unsigned int> problems(IOrganizer*) const override;
  QString description(unsigned int) const override;

  /**
   * @return the list of game names in this dependency requirement.
   */
  QStringList gameNames() const { return m_GameNames; }

protected:
  GameDependencyRequirement(QStringList const& gameNames);

  QStringList m_GameNames;
};

/**
 * @brief Diagnose dependency - This wrap a IPluginDiagnose into a plugin
 *     requirements.
 */
class DiagnoseRequirement : public IPluginRequirement {

  friend class PluginRequirementFactory;

public:

  std::vector<unsigned int> problems(IOrganizer*) const override;
  QString description(unsigned int) const override;

private:

  DiagnoseRequirement(const IPluginDiagnose* diagnose);

  const IPluginDiagnose* m_Diagnose;
};


/**
 * Factory for plugin requirements.
 */
class QDLLEXPORT PluginRequirementFactory {
public:

  /**
   * @brief Create a new plugin dependency. The requirement is met if one of the
   *     given plugin is enabled.
   *
   * If you want all plugins to be active, simply create multiple requirements.
   *
   * @param pluginNames Name of the plugin required.
   */
  static IPluginRequirement* pluginDependency(QStringList const& pluginNames);
  static IPluginRequirement* pluginDependency(QString const& pluginName) {
    return pluginDependency(QStringList{ pluginName });
  }

  /**
   * @brief Create a new plugin dependency. The requirement is met if the current
   *     game plugin matches one of the given name.
   *
   * @param gameName Name of the game required.
   *
   * @note This differ from makePluginDependency only for the message.
   */
  static IPluginRequirement* gameDependency(QStringList const& gameNames);
  static IPluginRequirement* gameDependency(QString const& gameName) {
    return gameDependency(QStringList{ gameName });
  }

  /**
   * @brief Create a requirement from the given diagnose plugin.
   *
   * @param diagnose The diagnose plugin.
   */
  static IPluginRequirement* diagnose(const IPluginDiagnose *diagnose);

  /**
   * @brief Create a generic requirement with the given checker and message.
   *
   * @param checker The function to use to check if the requirement is met (should
   *     return true if the requirement is met).
   * @param description The description to show user if the requirement is not met.
   */
  static IPluginRequirement* basic(std::function<bool(IOrganizer*)> const& checker, QString const description);

};

}

#endif