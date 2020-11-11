#include "pluginrequirements.h"

#include <QCoreApplication>

#include "imoinfo.h"
#include "iplugingame.h"
#include "iplugindiagnose.h"

using namespace MOBase;

// Plugin and Game dependencies

PluginDependencyRequirement::PluginDependencyRequirement(QStringList const& pluginNames) :
  m_PluginNames(pluginNames) { }

std::vector<unsigned int> PluginDependencyRequirement::problems(IOrganizer* o) const
{
  for (auto const& pluginName : m_PluginNames) {
    if (o->isPluginEnabled(pluginName)) {
      return {};
    }
  }
  return { 0 };
}

QString PluginDependencyRequirement::description(unsigned int) const
{
  if (m_PluginNames.size() > 1) {
    return QCoreApplication::translate(
      "PluginRequirement",
      "One of the following plugins must be enabled: %1.").arg(m_PluginNames.join(", "));
  }
  else {
    return QCoreApplication::translate(
      "PluginRequirement",
      "This plugin can only be enabled if the '%1' plugin is installed and enabled.").arg(m_PluginNames[0]);
  }
}

GameDependencyRequirement::GameDependencyRequirement(QStringList const& gameNames) :
  m_GameNames(gameNames) { }

std::vector<unsigned int> GameDependencyRequirement::problems(IOrganizer* o) const
{
  auto* game = o->managedGame();
  if (!game) {
    return { 0 };
  }

  QString gameName = game->gameName();
  for (auto const& pluginName : m_GameNames) {
    if (pluginName.compare(gameName, Qt::CaseInsensitive) == 0) {
      return {};
    }
  }
  return { 0 };
}

QString GameDependencyRequirement::description(unsigned int) const
{
  return QCoreApplication::translate(
    "PluginRequirement",
    "This plugin can only be enabled for the following game(s): %1.", "",
    m_GameNames.size()).arg(m_GameNames.join(", "));
}

// Diagnose requirements

DiagnoseRequirement::DiagnoseRequirement(const IPluginDiagnose *diagnose) :
  m_Diagnose(diagnose) { }

std::vector<unsigned int> DiagnoseRequirement::problems(IOrganizer*) const
{
  return m_Diagnose->activeProblems();
}

QString DiagnoseRequirement::description(unsigned int id) const
{
  return m_Diagnose->shortDescription(id);
}

// Basic requirements
class BasicPluginRequirement : public IPluginRequirement {
public:
  BasicPluginRequirement(std::function<bool(IOrganizer*)> const& checker, QString const description) :
    m_Checker(checker), m_Description(description) { }

  std::vector<unsigned int> problems(IOrganizer* o) const {
    if (m_Checker(o)) {
      return {};
    }
    return { 0 };
  }

  QString description(unsigned int) const {
    return m_Description;
  }

private:
  std::function<bool(IOrganizer*)> m_Checker;
  QString m_Description;

};

// Factory

IPluginRequirement* PluginRequirementFactory::pluginDependency(QStringList const& pluginNames)
{
  return new PluginDependencyRequirement(pluginNames);
}

IPluginRequirement* PluginRequirementFactory::gameDependency(QStringList const& pluginGameNames)
{
  return new GameDependencyRequirement(pluginGameNames);
}

IPluginRequirement* PluginRequirementFactory::diagnose(const IPluginDiagnose* diagnose)
{
  return new DiagnoseRequirement(diagnose);
}

IPluginRequirement* PluginRequirementFactory::basic(std::function<bool(IOrganizer*)> const& checker, QString const description)
{
  return new BasicPluginRequirement(checker, description);
}