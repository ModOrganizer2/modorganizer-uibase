#include "version.h"

#include <QRegularExpression>
#include <format>
#include <unordered_map>

#include "formatters.h"

// official semver regex
static const QRegularExpression s_SemVerStrictRegEx{
    R"(^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)"};

// for MO2, to match stuff like 1.2.3rc1 or 1.2.3a1+XXX
static const QRegularExpression s_SemVerMO2RegEx{
    R"(^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:(?P<type>dev|a|alpha|b|beta|rc)(?P<prerelease>0|[1-9]\d*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)"};

// match from value to release type
static const std::unordered_map<QString, MOBase::Version::ReleaseType>
    s_StringToRelease{
        {"dev", MOBase::Version::Development},    {"alpha", MOBase::Version::Alpha},
        {"alpha", MOBase::Version::Alpha},        {"a", MOBase::Version::Alpha},
        {"beta", MOBase::Version::Beta},          {"b", MOBase::Version::Beta},
        {"rc", MOBase::Version::ReleaseCandidate}};

namespace MOBase
{

namespace
{

  Version parseVersionSemVer(QString const& value)
  {
    const auto match = s_SemVerStrictRegEx.match(value);

    if (!match.hasMatch()) {
      throw InvalidVersionException(
          QString::fromStdString(std::format("invalid version string: '{}'", value)));
    }

    const auto major = match.captured("major").toInt();
    const auto minor = match.captured("minor").toInt();
    const auto patch = match.captured("patch").toInt();

    std::vector<std::variant<int, Version::ReleaseType>> prereleases;
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

    const auto buildMetadata = match.captured("buildmetadata").trimmed();

    return Version(major, minor, patch, prereleases, buildMetadata);
  }

  Version parseVersionMO2(QString const& value)
  {
    const auto match = s_SemVerMO2RegEx.match(value);

    if (!match.hasMatch()) {
      throw InvalidVersionException(
          QString::fromStdString(std::format("invalid version string: '{}'", value)));
    }

    const auto major = match.captured("major").toInt();
    const auto minor = match.captured("minor").toInt();
    const auto patch = match.captured("patch").toInt();

    std::vector<std::variant<int, Version::ReleaseType>> prereleases;
    if (match.hasCaptured("type")) {
      // unlike semver, the regex will only match valid values
      prereleases.push_back(s_StringToRelease.at(match.captured("type")));
      prereleases.push_back(match.captured("prerelease").toInt());
    }

    const auto buildMetadata = match.captured("buildmetadata").trimmed();

    return Version(major, minor, patch, prereleases, buildMetadata);
  }

}  // namespace

Version Version::parse(QString const& value, ParseMode mode)
{
  return mode == ParseMode::SemVer ? parseVersionSemVer(value) : parseVersionMO2(value);
}

Version::Version(int major, int minor, int patch, QString metadata)
    : Version(major, minor, patch, std::vector<std::variant<int, ReleaseType>>{},
              std::move(metadata))
{}

Version::Version(int major, int minor, int patch, ReleaseType type, QString metadata)
    : Version(major, minor, patch, std::vector<std::variant<int, ReleaseType>>{type},
              std::move(metadata))
{}

Version::Version(int major, int minor, int patch, ReleaseType type, int prerelease,
                 QString metadata)
    : Version(major, minor, patch, {type, prerelease}, std::move(metadata))
{}

Version::Version(int major, int minor, int patch,
                 std::vector<std::variant<int, ReleaseType>> prereleases,
                 QString metadata)
    : m_Major{major}, m_Minor{minor}, m_Patch{patch},
      m_PreReleases{std::move(prereleases)}, m_BuildMetadata{std::move(metadata)}
{}

QString Version::string() const
{
  auto value = std::format("{}.{}.{}", m_Major, m_Minor, m_Patch);

  if (!m_PreReleases.empty()) {
    value += "-";
    for (std::size_t i = 0; i < m_PreReleases.size(); ++i) {
      value += std::visit(
          [](auto const& pre) -> std::string {
            if constexpr (std::is_same_v<decltype(pre), ReleaseType const&>) {
              switch (pre) {
              case Development:
                return "dev";
              case Alpha:
                return "alpha";
              case Beta:
                return "beta";
              case ReleaseCandidate:
                return "rc";
              }
              return "";
            } else {
              return std::to_string(pre);
            }
          },
          m_PreReleases[i]);
      if (i < m_PreReleases.size() - 1) {
        value += ".";
      }
    }
  }

  if (!m_BuildMetadata.isEmpty()) {
    value += "+" + m_BuildMetadata.toStdString();
  }

  return QString::fromStdString(value);
}

std::strong_ordering operator<=>(const Version& lhs, const Version& rhs)
{
  auto mmp_cmp = std::forward_as_tuple(lhs.major(), lhs.minor(), lhs.patch()) <=>
                 std::forward_as_tuple(rhs.major(), rhs.minor(), rhs.patch());

  // major.minor.patch have precedence over everything else
  if (mmp_cmp != std::strong_ordering::equal) {
    return mmp_cmp;
  }

  // handle cases were one is a pre-release and not the other - the pre-release is
  // "less" than the release
  if (lhs.isPreRelease() && !rhs.isPreRelease()) {
    return std::strong_ordering::less;
  }

  if (!lhs.isPreRelease() && rhs.isPreRelease()) {
    return std::strong_ordering::greater;
  }

  // compare pre-release fields
  for (std::size_t i = 0;
       i < std::min(lhs.preReleases().size(), rhs.preReleases().size()); ++i) {

    const auto& lhsPre = lhs.preReleases()[i];
    const auto& rhsPre = rhs.preReleases()[i];

    // if one is alpha/beta/etc. and the other is numeric, the alpha/beta/etc. is lower
    // than the numeric one, which matches the index
    auto pre_cmp = lhsPre.index() <=> rhsPre.index();
    if (pre_cmp != std::strong_ordering::equal) {
      return pre_cmp;
    }

    // compare the actual values
    pre_cmp = lhsPre <=> rhsPre;
    if (pre_cmp != std::strong_ordering::equal) {
      return pre_cmp;
    }
  }

  // if we land here, the prefix of both pre were identical, so we need to compare the
  // size and the comparison is the reverse (i.e., the shorter wins)
  return lhs.preReleases().size() <=> rhs.preReleases().size();
}

}  // namespace MOBase