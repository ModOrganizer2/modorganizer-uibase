#ifndef UIBASE_REQUIREMENTS_H
#define UIBASE_REQUIREMENTS_H

#include <optional>

#include <QJsonValue>
#include <QString>

#include "dllimport.h"

namespace MOBase
{
class IOrganizer;
class ExtensionMetaData;

class ExtensionRequirementImpl;

// extension requirements
//
class QDLLEXPORT ExtensionRequirement
{
public:
  // type of requirement
  //
  enum class Type
  {
    // requirement on the version of MO2, might be ignored by user
    //
    VERSION,

    // require a specific game, cannot be ignore
    //
    GAME,

    // require another extension, cannot be ignore
    //
    DEPENDENCY
  };

  using enum Type;

public:
  // check if the requirement is met
  //
  bool check(IOrganizer* organizer) const;

  ~ExtensionRequirement();

private:
  friend class ExtensionRequirementFactory;

  ExtensionRequirement(std::shared_ptr<ExtensionRequirementImpl> impl);

  std::shared_ptr<ExtensionRequirementImpl> m_Impl;
};

// factory for requirements
//
class QDLLEXPORT ExtensionRequirementFactory
{
public:
  // extract requirements from the given metadata
  //
  static std::vector<ExtensionRequirement>
  parseRequirements(const ExtensionMetaData& metadata);

private:
};

}  // namespace MOBase

#endif