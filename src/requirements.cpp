#include "requirements.h"

#include <QJsonArray>

#include "extension.h"
#include "imoinfo.h"
#include "log.h"

namespace MOBase
{

class ExtensionRequirementImpl
{};

}  // namespace MOBase

using namespace MOBase;

ExtensionRequirement::ExtensionRequirement(
    std::shared_ptr<ExtensionRequirementImpl> impl)
    : m_Impl{std::move(impl)}
{}

ExtensionRequirement::~ExtensionRequirement() = default;

bool ExtensionRequirement::check(IOrganizer* organizer) const
{
  return true;
}

std::vector<ExtensionRequirement>
ExtensionRequirementFactory::parseRequirements(const ExtensionMetaData& metadata)
{
  const auto json_requirements = metadata.json()["requirements"];

  if (!json_requirements.isArray()) {
    log::warn("expected array of requirements for extension '{}', found '{}'",
              metadata.identifier(), json_requirements.type());
    return {};
  }

  std::vector<ExtensionRequirement> requirements;
  for (const auto& json_requirement : json_requirements.toArray()) {
  }

  return requirements;
}