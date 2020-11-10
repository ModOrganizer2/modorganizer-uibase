#ifndef PLUGINREQUIREMENTS_H
#define PLUGINREQUIREMENTS_H

#include <functional>

#include <QStringList>

namespace MOBase {

class IOrganizer;
class IPluginDiagnose;

/**
 * @brief The interface for plugin requirements.
 */
class PluginRequirement {
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

  virtual ~PluginRequirement() { }
};

/**
 * @brief Plugin dependency - The requirement is met if one of the
 *     given plugin is active.
 */
class PluginDependencyRequirement : public PluginRequirement {

  friend class PluginRequirementFactory;

public:

  std::vector<unsigned int> problems(IOrganizer*) const override;
  QString description(unsigned int) const override;

protected:
  PluginDependencyRequirement(QStringList const& pluginNames);

  QStringList m_PluginNames;
};

/**
 * @brief Game dependency - The requirement is met if the active game
 *     is one of the specified game.
 */
class GameDependencyRequirement : public PluginDependencyRequirement {

  friend class PluginRequirementFactory;

public:
  QString description(unsigned int) const override;

protected:
  using PluginDependencyRequirement::PluginDependencyRequirement;
};

/**
 * @brief Diagnose dependency - This wrap a IPluginDiagnose into a plugin
 *     requirements.
 */
class DiagnoseRequirement : public PluginRequirement {

  friend class PluginRequirementFactory;

public:

  std::vector<unsigned int> problems(IOrganizer*) const override;
  QString description(unsigned int) const override;

private:

  DiagnoseRequirement(IPluginDiagnose* diagnose);

  IPluginDiagnose* m_Diagnose;
};


/**
 * Factory for plugin requirements.
 */
class PluginRequirementFactory {
public:

  /**
   * @brief Create a new plugin dependency. The requirement is met if one of the
   *     given plugin is enabled.
   *
   * If you want all plugins to be active, simply create multiple requirements.
   *
   * @param pluginNames Name of the plugin required.
   */
  static PluginRequirement* pluginDependency(QStringList const& pluginNames);

  /**
   * @brief Create a new plugin dependency. The requirement is met if one of the
   *     given plugin is enabled.
   *
   * If you want all plugins to be active, simply create multiple requirements.
   *
   * @param pluginGameNames Name of the plugin required.
   *
   * @note This differ from makePluginDependency only for the message.
   */
  static PluginRequirement* gameDependency(QStringList const& pluginGameNames);

  /**
   * @brief Create a requirement from the given diagnose plugin.
   *
   * @param diagnose The diagnose plugin.
   */
  static PluginRequirement* diagnose(IPluginDiagnose *diagnose);

  /**
   * @brief Create a generic requirement with the given checker and message.
   *
   * @param checker The function to use to check if the requirement is met (should
   *     return true if the requirement is met).
   * @param description The description to show user if the requirement is not met.
   */
  static PluginRequirement* basic(std::function<bool(IOrganizer*)> const& checker, QString const description);

};

}

#endif