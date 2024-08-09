#include "extensions/requirements.h"

#include <QJsonArray>

#include "extensions/extension.h"
#include "extensions/iextensionlist.h"
#include "extensions/versionconstraints.h"
#include "imoinfo.h"
#include "log.h"

namespace MOBase
{

class ExtensionRequirementImpl
{
public:
  using Type = ExtensionRequirement::Type;

public:
  virtual bool check(IOrganizer* organizer) const = 0;
  virtual Type type() const                       = 0;
  virtual QString string() const                  = 0;
  virtual ~ExtensionRequirementImpl()             = default;
};

// requirement for the version of MO2 itself
//
class CoreVersionExtensionRequirement : public ExtensionRequirementImpl
{
public:
  CoreVersionExtensionRequirement(VersionConstraints const& constraints)
      : m_Constraints{constraints}
  {}

  bool check(IOrganizer* organizer) const override
  {
    return m_Constraints.matches(organizer->version());
  }

  Type type() const override { return Type::VERSION; }

  QString string() const override
  {
    return QString("ModOrganizer2 %1").arg(m_Constraints.string());
  }

private:
  VersionConstraints m_Constraints;
};

// requirement for another extension
//
class DependencyExtensionRequirement : public ExtensionRequirementImpl
{
public:
  DependencyExtensionRequirement(QString const& extension,
                                 VersionConstraints const& constraints)
      : m_Extension{extension}, m_Constraints{constraints}
  {}

  bool check(IOrganizer* organizer) const override
  {
    return organizer->extensionList().enabled(m_Extension) &&
           m_Constraints.matches(
               organizer->extensionList().get(m_Extension).metadata().version());
  }

  Type type() const override { return Type::DEPENDENCY; }

  QString string() const override
  {
    return QString("%1 %2").arg(m_Extension, m_Constraints.string());
  }

private:
  QString m_Extension;
  VersionConstraints m_Constraints;
};

// requirement for games
//
class GameExtensionRequirement : public ExtensionRequirementImpl
{
public:
  GameExtensionRequirement(QStringList const& games) : m_Games{games} {}

  bool check(IOrganizer* organizer) const override
  {
    return organizer->managedGame() &&
           m_Games.contains(organizer->managedGame()->gameName());
  }

  Type type() const override { return Type::GAME; }

  QString string() const override { return m_Games.join(", "); }

private:
  QStringList m_Games;
};

}  // namespace MOBase

using namespace MOBase;

ExtensionRequirement::ExtensionRequirement(
    std::shared_ptr<ExtensionRequirementImpl> impl)
    : m_Impl{std::move(impl)}
{}

ExtensionRequirement::~ExtensionRequirement() = default;

bool ExtensionRequirement::check(IOrganizer* organizer) const
{
  return m_Impl->check(organizer);
}

ExtensionRequirement::Type ExtensionRequirement::type() const
{
  return m_Impl->type();
}

QString ExtensionRequirement::string() const
{
  return m_Impl->string();
}

namespace
{
std::optional<ExtensionRequirement::Type> parseType(QString const& value)
{
  std::map<QString, ExtensionRequirement::Type> stringToTypes{
      {"game", ExtensionRequirement::Type::GAME},
      {"extension", ExtensionRequirement::Type::DEPENDENCY},
      {"version", ExtensionRequirement::Type::VERSION}};

  std::optional<ExtensionRequirement::Type> type;
  for (auto& [k, v] : stringToTypes) {
    if (k.compare(value, Qt::CaseInsensitive) == 0) {
      type = v;
      break;
    }
  }

  return type;
}
}  // namespace

std::vector<ExtensionRequirement>
ExtensionRequirementFactory::parseRequirements(const QJsonValue& json_requirements)
{
  if (!json_requirements.isArray()) {
    throw InvalidRequirementsException("expected an array of requirements");
  }

  std::vector<ExtensionRequirement> requirements;
  for (const auto& json_requirement : json_requirements.toArray()) {
    if (!json_requirement.isObject()) {
      throw InvalidRequirementException("invalid requirement");
    }

    auto json_object = json_requirement.toObject();

    const auto type = parseType(json_object["type"].toString());
    if (!type.has_value()) {
      throw InvalidRequirementException("missing requirement type");
    }

    try {
      switch (*type) {
      case ExtensionRequirement::Type::GAME:
        if (!json_object.contains("games") || !json_object["games"].isArray()) {
          throw InvalidRequirementException("invalid requirement");
        }
        requirements.push_back(
            ExtensionRequirement(std::make_shared<GameExtensionRequirement>(
                json_object["games"].toVariant().toStringList())));
        break;
      case ExtensionRequirement::Type::DEPENDENCY:
        requirements.push_back(
            ExtensionRequirement(std::make_shared<DependencyExtensionRequirement>(
                json_object["extension"].toString(),
                VersionConstraints::parse(json_object["version"].toString(),
                                          Version::ParseMode::SemVer))));
        break;
      case ExtensionRequirement::Type::VERSION:
        requirements.push_back(ExtensionRequirement(
            std::make_shared<CoreVersionExtensionRequirement>(VersionConstraints::parse(
                json_object["version"].toString(), Version::ParseMode::MO2))));
        break;
      }
    } catch (InvalidConstraintException const& ex) {
      throw InvalidRequirementException(
          std::format("invalid requirement constraints: {}", ex.what()));
    }
  }

  return requirements;
}
