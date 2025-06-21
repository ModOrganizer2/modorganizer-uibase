#ifndef UIBASE_REQUIREMENTS_H
#define UIBASE_REQUIREMENTS_H

#include <optional>

#include <QJsonValue>
#include <QString>

#include "../dllimport.h"
#include "../exceptions.h"

namespace MOBase
{
class IOrganizer;
class ExtensionMetaData;

class InvalidRequirementException : public Exception
{
public:
  using Exception::Exception;
};

class InvalidRequirementsException : public Exception
{
public:
  using Exception::Exception;
};

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

  // retrieve the type of this extension
  //
  Type type() const;

  // retrieve a textual representation of this requirement, e.g. "ModOrganizer 2.5.4"
  // for a requirement that requires MO2 2.5.4
  //
  QString string() const;

public:
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
  parseRequirements(const QJsonValue& json_requirements);

private:
};

}  // namespace MOBase

#endif
