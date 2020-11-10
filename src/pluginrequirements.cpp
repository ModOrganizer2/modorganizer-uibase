#include "pluginrequirements.h"

#include <QCoreApplication>

#include "imoinfo.h"
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
  return QCoreApplication::translate(
    "PluginRequirement",
    "One of the following plugins must be enabled: %s").arg(m_PluginNames.join(", "));
}

QString GameDependencyRequirement::description(unsigned int) const
{
  return QCoreApplication::translate(
    "PluginRequirement",
    "This plugin can only be enabled for the following games: %s").arg(m_PluginNames.join(", "));
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
class BasicPluginRequirement : public PluginRequirement {
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

PluginRequirement* PluginRequirementFactory::pluginDependency(QStringList const& pluginNames)
{
  return new PluginDependencyRequirement(pluginNames);
}

PluginRequirement* PluginRequirementFactory::gameDependency(QStringList const& pluginGameNames)
{
  return new GameDependencyRequirement(pluginGameNames);
}

PluginRequirement* PluginRequirementFactory::diagnose(const IPluginDiagnose* diagnose)
{
  return new DiagnoseRequirement(diagnose);
}

PluginRequirement* PluginRequirementFactory::basic(std::function<bool(IOrganizer*)> const& checker, QString const description)
{
  return new BasicPluginRequirement(checker, description);
}