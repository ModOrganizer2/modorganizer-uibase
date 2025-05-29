#include "extensions/versionconstraints.h"

#include "formatters.h"

using VersionCompareFunction = bool (*)(MOBase::Version const& lhs,
                                        MOBase::Version const& rhs);

// official semver regex
static const QRegularExpression s_ConstraintStrictRegEx{
    R"(^(?P<constraint>>=|<=|<|>|!=|==|\^|~)?\s*(?P<major>0|[1-9*]\d*)(?:\.(?P<minor>0|[1-9*]\d*)(?:\.(?P<patch>0|[1-9*]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?)?)?$)"};

// for MO2, to match stuff like 1.2.3rc1 or v1.2.3a1+XXX
static const QRegularExpression s_ConstraintMO2RegEx{
    R"(^(?P<constraint>>=|<=|<|>|!=|\^|~)?\s*(?P<major>0|[1-9*]\d*)(?:\.(?P<minor>0|[1-9*]\d*)(?:\.(?P<patch>0|[1-9*]\d*)(?:\.(?P<subpatch>0|[1-9*]\d*))?(?:(?P<type>dev|a|alpha|b|beta|rc)(?P<prerelease>0|[1-9](?:[.0-9])*))?)?)?$)"};

// match from value to release type
static const std::unordered_map<QString, MOBase::Version::ReleaseType>
    s_StringToRelease{{"dev", MOBase::Version::Development},
                      {"alpha", MOBase::Version::Alpha},
                      {"a", MOBase::Version::Alpha},
                      {"beta", MOBase::Version::Beta},
                      {"b", MOBase::Version::Beta},
                      {"rc", MOBase::Version::ReleaseCandidate}};

#define _COMPARE_PAIR(OP)                                                              \
  {#OP, +[](MOBase::Version const& lhs, MOBase::Version const& rhs) {                  \
     return lhs OP rhs;                                                                \
   }}

static const std::unordered_map<QString, VersionCompareFunction> s_CompareToFunction{
    _COMPARE_PAIR(>),  _COMPARE_PAIR(>=), _COMPARE_PAIR(<),
    _COMPARE_PAIR(<=), _COMPARE_PAIR(!=), _COMPARE_PAIR(==)};

#undef _COMPARE_PAIR

namespace MOBase
{

class VersionConstraintImpl
{
public:
  virtual bool matches(Version const& version) const = 0;
  virtual ~VersionConstraintImpl()                   = default;
};

// version constraint for a range with lower bound included and upper bound excluded,
// typically used for tilde, caret and wilcard constraints
//
class RangeVersionConstraint : public VersionConstraintImpl
{
public:
  RangeVersionConstraint(Version const& min, Version const& max)
      : m_Min{min}, m_Max{max}
  {}

  bool matches(Version const& version) const override
  {
    return m_Min <= version && version < m_Max;
  }

private:
  Version m_Min, m_Max;
};

// version constraint for inequality and equality constraint
//
class InequalityVersionConstraint : public VersionConstraintImpl
{

public:
  InequalityVersionConstraint(Version const& target, VersionCompareFunction compare)
      : m_Target{target}, m_Compare{compare}
  {}

  bool matches(Version const& version) const override
  {
    return m_Compare(version, m_Target);
  }

private:
  Version m_Target;
  VersionCompareFunction m_Compare;
};

VersionConstraint VersionConstraint::parse(QString const& value,
                                           Version::ParseMode mode)
{
  const auto& regex = mode == Version::ParseMode::SemVer ? s_ConstraintStrictRegEx
                                                         : s_ConstraintMO2RegEx;

  const auto match = regex.match(value);
  if (!match.hasMatch()) {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }

  const auto constraint = match.captured("constraint");

  const auto major_s    = match.captured("major");
  const auto minor_s    = match.captured("minor");
  const auto patch_s    = match.captured("patch");
  const auto subpatch_s = match.captured("subpatch");

  const auto wildcard =
      major_s == "*" || minor_s == "*" || patch_s == "*" || subpatch_s == "*";
  const auto tilde = match.captured("constraint") == "~";
  const auto caret = match.captured("constraint") == "^";

  // cannot use wildcard with a constraint
  if (wildcard && !constraint.isEmpty()) {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }

  // cannot use pre-release with wilcard, tilde or caret constraint
  if ((wildcard || tilde || caret) && match.hasCaptured("prerelease")) {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }

  // if a part has a wildcard, lower part should be missing or wildcard (e.g., 2.*.3
  // is invalid)
  if (major_s == "*" && !minor_s.isEmpty() && minor_s != "*") {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }
  if (minor_s == "*" && !patch_s.isEmpty() && patch_s != "*") {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }
  if (patch_s == "*" && !subpatch_s.isEmpty() && subpatch_s != "*") {
    throw InvalidConstraintException(
        QString::fromStdString(std::format("invalid constraint string: '{}'", value)));
  }

  std::vector<std::variant<int, Version::ReleaseType>> prereleases;
  if (mode == Version::ParseMode::SemVer) {
    for (auto& part : match.captured("prerelease")
                          .split(".", Qt::SplitBehaviorFlags::SkipEmptyParts)) {
      // try to extract an int
      bool ok             = true;
      const auto intValue = part.toInt(&ok);
      if (ok) {
        prereleases.push_back(intValue);
        continue;
      }

      // check if we have a valid prerelease type
      const auto it = s_StringToRelease.find(part.toLower());
      if (it == s_StringToRelease.end()) {
        throw InvalidVersionException(
            QString::fromStdString(std::format("invalid prerelease type: '{}'", part)));
      }

      prereleases.push_back(it->second);
    }
  } else {
    prereleases.push_back(s_StringToRelease.at(match.captured("type")));

    // for version with decimal point, e.g., 2.4.1rc1.1, we split the components into
    // pre-release components to get {rc, 1, 1} - this works fine since {rc, 1} < {rc,
    // 1, 1}
    //
    for (const auto& preVersion :
         match.captured("prerelease").split(".", Qt::SkipEmptyParts)) {
      prereleases.push_back(preVersion.toInt());
    }
  }

  constexpr auto max_int = std::numeric_limits<int>::max();

  std::shared_ptr<VersionConstraintImpl> impl;

  if (wildcard || caret || tilde) {

    // you can get more information at
    // https://python-poetry.org/docs/dependency-specification/

    // note that the only case where all 4 xxxOk is false is for '*'
    //
    bool majorOk, minorOk, patchOk, subpatchOk;
    auto major = major_s.toInt(&majorOk), minor = minor_s.toInt(&minorOk),
         patch = patch_s.toInt(&patchOk), subpatch = subpatch_s.toInt(&subpatchOk);

    // the lower bound is always the actual version with missing or wildcard components
    // set to 0, e.g.
    // - 2.3.* -> >= 2.3.0
    // - ^1    -> >= 1.0.0
    // - ^0.3  -> >= 0.3.0
    // - ~1.2  -> >= 1.2.0
    const Version min = Version(major, minor, patch, subpatch);

    // the upper bound is a bit more complicated to compute
    Version max = Version(max_int, max_int, max_int, max_int);

    if (wildcard) {
      // for wildcard, we increment the last non-wildcard character by one
      //
      if (majorOk && minorOk && patchOk) {
        max = Version(major, minor, patch + 1);
      } else if (majorOk && minorOk) {
        max = Version(major, minor + 1, 0);
      } else if (majorOk) {
        max = Version(major + 1, 0, 0);
      } else {
        max = Version(max_int, max_int, max_int, max_int);
      }
    } else if (caret) {
      // TODO: clean this...

      if (!minorOk && !patchOk && !subpatchOk) {
        max = Version(major + 1, 0, 0);
      } else if (!patchOk && !subpatchOk) {
        if (major == 0) {
          max = Version(major, minor + 1, 0);
        } else {
          max = Version(major + 1, 0, 0);
        }
      } else if (!subpatchOk) {
        if (major == 0 && minor == 0) {
          max = Version(major, minor, patch + 1);
        } else if (major == 0) {
          max = Version(major, minor + 1, 0);
        } else {
          max = Version(major + 1, 0, 0);
        }
      } else {
        if (major == 0 && minor == 0 && patch == 0 && subpatch == 0) {
          max = min;  // this creates an impossible range (>= 0, < 0), but is expected
        } else if (major == 0 && minor == 0 && patch == 0) {
          max = Version(major, minor, patch, subpatch + 1);
        } else if (major == 0 && minor == 0) {
          max = Version(major, minor, patch + 1, 0);
        } else if (major == 0) {
          max = Version(major, minor + 1, 0);
        } else {
          max = Version(major + 1, 0, 0);
        }
      }

    } else if (tilde) {
      if (minorOk && patchOk && subpatchOk) {
        max = Version(major, minor, patch, subpatch + 1);
      } else if (minorOk && patchOk) {
        max = Version(major, minor, patch + 1);
      } else if (minorOk) {
        max = Version(major, minor + 1, 0);
      } else {
        max = Version(major + 1, 0, 0);
      }
    }

    impl = std::make_shared<RangeVersionConstraint>(min, max);

  } else {
    auto op = match.captured("constraint");
    if (op.isEmpty()) {
      op = "==";
    }
    impl = std::make_shared<InequalityVersionConstraint>(
        Version(major_s.toInt(), minor_s.toInt(), patch_s.toInt(), subpatch_s.toInt(),
                std::move(prereleases)),
        s_CompareToFunction.at(op));
  }

  return VersionConstraint(std::move(impl));
}

VersionConstraint::VersionConstraint(std::shared_ptr<VersionConstraintImpl> impl)
    : m_Impl{std::move(impl)}
{}

VersionConstraint::~VersionConstraint() = default;

bool VersionConstraint::matches(Version const& version) const
{
  return m_Impl->matches(version);
}

VersionConstraints VersionConstraints::parse(QString const& value,
                                             Version::ParseMode mode)
{
  std::vector<VersionConstraint> constraints;
  auto parts = value.split(",");
  for (auto& part : parts) {
    // replace the part in-place to create a proper representation
    part = part.simplified().replace(" ", "");

    constraints.push_back(VersionConstraint::parse(part, mode));
  }
  return VersionConstraints(parts.join(", "), std::move(constraints));
}

bool VersionConstraints::matches(Version const& version) const
{
  return std::all_of(m_Constraints.begin(), m_Constraints.end(),
                     [version](const auto& constraint) {
                       return constraint.matches(version);
                     });
}

VersionConstraints::VersionConstraints(QString const& repr,
                                       std::vector<VersionConstraint> checkers)
    : m_Repr{repr}, m_Constraints{std::move(checkers)}
{}

}  // namespace MOBase
