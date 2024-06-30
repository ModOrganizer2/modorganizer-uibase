#pragma once

#include <compare>
#include <string>
#include <variant>
#include <vector>

#include "dllimport.h"
#include "exceptions.h"

namespace MOBase
{

class InvalidVersionException : public Exception
{
public:
  using Exception::Exception;
};

// class representing a SemVer object, see https://semver.org/
//
// unlike VersionInfo, this class is immutable and only hold valid versions
//
class QDLLEXPORT Version
{
public:
  enum class ParseMode
  {
    // official semver parsing
    //
    SemVer,

    // MO2 parsing, e.g., 2.5.1rc1 - this either parse a string with no pre-release
    // information (e.g. 2.5.1) or with a single pre-release + a version (e.g., 2.5.1a1
    // or 2.5.2rc1)
    //
    MO2
  };

  enum class ReleaseType
  {
    Development,       // -dev
    Alpha,             // -alpha, -a
    Beta,              // -beta, -b
    ReleaseCandidate,  // -rc
  };
  using enum ReleaseType;

public:  // parsing
  // parse version from the given string, throw InvalidVersionException if the string
  // cannot be parsed
  //
  static Version parse(QString const& value, ParseMode mode = ParseMode::SemVer);

public:  // constructor
  Version(int major, int minor, int patch, QString metadata = {});
  Version(int major, int minor, int patch, ReleaseType type, QString metadata = {});
  Version(int major, int minor, int patch, ReleaseType type, int prerelease,
          QString metadata = {});
  Version(int major, int minor, int patch,
          std::vector<std::variant<int, ReleaseType>> prereleases,
          QString metadata = {});

public:  // special member functions
  Version(const Version&) = default;
  Version(Version&&)      = default;

  Version& operator=(const Version&) = default;
  Version& operator=(Version&&)      = default;

public:
  // check if this version corresponds to a pre-release version (dev, alpha, beta, etc.)
  //
  bool isPreRelease() const { return !m_PreReleases.empty(); }

  // retrieve major, minor and patch of this version
  //
  int major() const { return m_Major; }
  int minor() const { return m_Minor; }
  int patch() const { return m_Patch; }

  // retrieve pre-releases information for this version
  //
  const auto& preReleases() const { return m_PreReleases; }

  // retrieve build metadata, if any, otherwise return an empty string
  //
  const auto& buildMetadata() const { return m_BuildMetadata; }

  // convert this version to a semver string
  //
  QString string() const;

private:
  // major.minor.patch
  int m_Major, m_Minor, m_Patch;

  // pre-release information
  std::vector<std::variant<int, ReleaseType>> m_PreReleases;

  // metadata
  QString m_BuildMetadata;
};

QDLLEXPORT std::strong_ordering operator<=>(const Version& lhs, const Version& rhs);

inline bool operator==(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) == 0;
}
inline bool operator!=(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) != 0;
}
inline bool operator<(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) < 0;
}
inline bool operator>(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) > 0;
}
inline bool operator<=(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) <= 0;
}
inline bool operator>=(const Version& lhs, const Version& rhs)
{
  return (lhs <=> rhs) >= 0;
}

}  // namespace MOBase

template <class CharT>
struct std::formatter<MOBase::Version, CharT> : std::formatter<QString, CharT>
{
  template <class FmtContext>
  FmtContext::iterator format(const MOBase::Version& v, FmtContext& ctx) const
  {
    return std::formatter<QString, CharT>::format(v.string(), ctx);
  }
};