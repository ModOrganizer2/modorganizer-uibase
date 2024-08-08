#pragma once

#include <optional>

#include <QString>

#include "../versioning.h"

namespace MOBase
{
class InvalidConstraintException : public Exception
{
public:
  using Exception::Exception;
};

class VersionConstraintImpl;

// class representing a version constraint, e.g. "2.3.*" or ">=2.4"
//
class QDLLEXPORT VersionConstraint
{
public:
  // wildcard placeholder for major/minor/patch/subpatch when constructing wildcard
  //
  static constexpr int WILDCARD = -1;

public:
  // parse a constraint from the given string
  //
  static VersionConstraint parse(QString const& value, Version::ParseMode mode);

public:
  // check if the given version matches this constraint
  //
  bool matches(Version const& version) const;

public:
  ~VersionConstraint();

private:
  VersionConstraint(std::shared_ptr<VersionConstraintImpl> impl);

  std::shared_ptr<VersionConstraintImpl> m_Impl;
};

// class representing a set of version constraints, usually from dependency
// requirements e.g. "2.3.*", or ">= 2.4, <2.5"
//
class QDLLEXPORT VersionConstraints
{
public:
  // parse a set of constraints from the given string
  //
  static VersionConstraints parse(QString const& value, Version::ParseMode mode);

public:
  // construct a set of constraints
  //
  VersionConstraints(std::vector<VersionConstraint> constraints);

  // check if the given version matches the set of constraints
  //
  bool matches(Version const& version) const;

private:
  std::vector<VersionConstraint> m_Constraints;
};

}  // namespace MOBase
