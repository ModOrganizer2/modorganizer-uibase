#pragma warning(push)
#pragma warning(disable : 4668)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <QString>
#include <vector>

#include <uibase/extensions/versionconstraints.h>
#include <uibase/versioning.h>

#include <format>

using namespace MOBase;

using enum Version::ReleaseType;
using ParseMode = Version::ParseMode;

TEST(VersioningTest, VersionParse)
{
  // TODO: add exceptions test

  // semver
  ASSERT_EQ(Version(1, 0, 0), Version::parse("1.0.0"));
  ASSERT_EQ(Version(1, 0, 0, Development, 1), Version::parse("1.0.0-dev.1"));
  ASSERT_EQ(Version(1, 0, 0, Development, 2), Version::parse("1.0.0-dev.2"));
  ASSERT_EQ(Version(1, 0, 0, Alpha), Version::parse("1.0.0-a"));
  ASSERT_EQ(Version(1, 0, 0, Alpha), Version::parse("1.0.0-alpha"));
  ASSERT_EQ(Version(1, 0, 0, 0, {Alpha, 1, Beta}), Version::parse("1.0.0-alpha.1.b"));
  ASSERT_EQ(Version(1, 0, 0, Beta, 2), Version::parse("1.0.0-beta.2"));
  ASSERT_EQ(Version(2, 5, 2, ReleaseCandidate, 1), Version::parse("2.5.2-rc.1"));

  // mo2
  ASSERT_EQ(Version(1, 0, 0), Version::parse("1.0.0", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Development, 1),
            Version::parse("1.0.0dev1", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Development, 2),
            Version::parse("1.0.0dev2", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Alpha, 1), Version::parse("1.0.0a1", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Alpha, 1), Version::parse("1.0.0alpha1", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Beta, 2), Version::parse("1.0.0beta2", ParseMode::MO2));
  ASSERT_EQ(Version(1, 0, 0, Beta, 2), Version::parse("1.0.0beta2", ParseMode::MO2));
  ASSERT_EQ(Version(2, 4, 1, 0, {ReleaseCandidate, 1, 1}),
            Version::parse("2.4.1rc1.1", ParseMode::MO2));
  ASSERT_EQ(Version(2, 2, 2, 1, Beta, 2),
            Version::parse("2.2.2.1beta2", ParseMode::MO2));
  ASSERT_EQ(Version(2, 5, 2, ReleaseCandidate, 1),
            Version::parse("v2.5.2rc1", ParseMode::MO2));
  ASSERT_EQ(Version(2, 5, 2, ReleaseCandidate, 2),
            Version::parse("2.5.2rc2", ParseMode::MO2));
}

TEST(VersioningTest, VersionString)
{
  ASSERT_EQ("1.0.0", Version(1, 0, 0).string());
  ASSERT_EQ("1.0.0-dev.1", Version(1, 0, 0, Development, 1).string());
  ASSERT_EQ("1.0.0-dev.2", Version(1, 0, 0, Development, 2).string());
  ASSERT_EQ("1.0.0-alpha", Version(1, 0, 0, Alpha).string());
  ASSERT_EQ("1.0.0-alpha.1.beta", Version(1, 0, 0, 0, {Alpha, 1, Beta}).string());
  ASSERT_EQ("1.0.0-beta.2", Version(1, 0, 0, Beta, 2).string());
  ASSERT_EQ("2.5.2-rc.1", Version(2, 5, 2, ReleaseCandidate, 1).string());
  ASSERT_EQ("2.5.2rc1",
            Version(2, 5, 2, ReleaseCandidate, 1).string(Version::FormatCondensed));
}

TEST(VersioningTest, VersionCompare)
{
  // shortcut
  using v = Version;

  // test from https://semver.org/
  ASSERT_TRUE(v(1, 0, 0) < v(2, 0, 0));
  ASSERT_TRUE(v(2, 0, 0) < v(2, 1, 0));
  ASSERT_TRUE(v(2, 1, 0) < v(2, 1, 1));

  ASSERT_TRUE(v(1, 0, 0, Alpha) < v(1, 0, 0, Alpha, 1));
  ASSERT_TRUE(v(1, 0, 0, Alpha, 1) < v(1, 0, 0, 0, {Alpha, Beta}));
  ASSERT_TRUE(v(1, 0, 0, 0, {Alpha, Beta}) < v(1, 0, 0, 1));
  ASSERT_TRUE(v(1, 0, 0, Beta) < v(1, 0, 0, Beta, 2));
  ASSERT_TRUE(v(1, 0, 0, Beta, 2) < v(1, 0, 0, Beta, 11));
  ASSERT_TRUE(v(1, 0, 0, Beta, 11) < v(1, 0, 0, ReleaseCandidate, 1));
  ASSERT_TRUE(v(1, 0, 0, ReleaseCandidate, 0) < v(1, 0, 0));

  ASSERT_TRUE(v(2, 4, 1, 0, {ReleaseCandidate, 1, 0}) ==
              v(2, 4, 1, ReleaseCandidate, 1));
  ASSERT_TRUE(v(2, 4, 1, 0, {ReleaseCandidate, 1, 0}) <
              v(2, 4, 1, 0, {ReleaseCandidate, 1, 1}));
  ASSERT_TRUE(v(2, 4, 1, ReleaseCandidate, 1) <
              v(2, 4, 1, 0, {ReleaseCandidate, 1, 1}));
  ASSERT_TRUE(v(1, 0, 0) < v(2, 0, 0, Alpha));
}

TEST(VersioningTest, VersionConstraintTest)
{
  // shortcut
  using v = Version;

  constexpr auto MAX = std::numeric_limits<int>::max();

  auto check = [](const QString& constraint, Version const& v,
                  Version::ParseMode mode = Version::ParseMode::SemVer) {
    return VersionConstraint::parse(constraint, mode).matches(v);
  };

  // inequality

  ASSERT_TRUE(check("2.5.2", v(2, 5, 2)));
  ASSERT_FALSE(check("2.5.3", v(2, 5, 2)));

  ASSERT_TRUE(check(">2.5", v(2, 5, 1)));
  ASSERT_TRUE(check(">2.5.2", v(2, 5, 3)));
  ASSERT_FALSE(check(">2.5.2", v(2, 5, 2)));
  ASSERT_FALSE(check(">2.5.3", v(2, 5, 2)));

  ASSERT_TRUE(check("<2.5", v(2, 4, MAX)));

  // wilcard

  ASSERT_TRUE(check("*", v(2, 4, MAX)));

  ASSERT_TRUE(check("2.4.*", v(2, 4, 0)));
  ASSERT_TRUE(check("2.4.*", v(2, 4, MAX)));
  ASSERT_FALSE(check("2.4.*", v(2, 3, MAX)));
  ASSERT_FALSE(check("2.4.*", v(2, 5, 0)));

  // caret

  ASSERT_TRUE(check("^1.2.3", v(1, 2, 3)));
  ASSERT_TRUE(check("^1.2.3", v(1, 2, 4)));
  ASSERT_TRUE(check("^1.2.3", v(1, 3, 1)));
  ASSERT_TRUE(check("^1.2.3", v(1, MAX, 5)));
  ASSERT_FALSE(check("^1.2.3", v(1, 2, 2, MAX)));
  ASSERT_FALSE(check("^1.2.3", v(1, 1, 0)));
  ASSERT_FALSE(check("^1.2.3", v(2, 0, 0)));

  ASSERT_TRUE(check("^1.2", v(1, 2, 0)));
  ASSERT_TRUE(check("^1.2", v(1, 2, 4)));
  ASSERT_TRUE(check("^1.2", v(1, 3, 1)));
  ASSERT_TRUE(check("^1.2", v(1, 9, 5)));
  ASSERT_FALSE(check("^1.2", v(1, 1, MAX)));
  ASSERT_FALSE(check("^1.2", v(1, 1, 0)));
  ASSERT_FALSE(check("^1.2", v(2, 0, 0)));

  ASSERT_TRUE(check("^1", v(1, 0, 0)));
  ASSERT_TRUE(check("^1", v(1, 2, 4)));
  ASSERT_TRUE(check("^1", v(1, 3, 1)));
  ASSERT_TRUE(check("^1", v(1, 9, 5)));
  ASSERT_FALSE(check("^1", v(0, MAX, MAX)));
  ASSERT_FALSE(check("^1", v(0, MAX, 0)));
  ASSERT_FALSE(check("^1", v(2, 0, 0)));

  ASSERT_TRUE(check("^0.2.3", v(0, 2, 3)));
  ASSERT_TRUE(check("^0.2.3", v(0, 2, MAX)));
  ASSERT_FALSE(check("^0.2.3", v(0, 1, MAX)));
  ASSERT_FALSE(check("^0.2.3", v(0, 3, 0)));

  ASSERT_TRUE(check("^0.0.3", v(0, 0, 3)));
  ASSERT_TRUE(check("^0.0.3", v(0, 0, 3, MAX)));
  ASSERT_FALSE(check("^0.0.3", v(0, 0, 2, MAX)));
  ASSERT_FALSE(check("^0.0.3", v(0, 0, 4)));

  ASSERT_TRUE(check("^0.0", v(0, 0, 0)));
  ASSERT_TRUE(check("^0.0", v(0, 0, MAX)));
  ASSERT_FALSE(check("^0.0", v(0, 1, 0)));

  ASSERT_TRUE(check("^0", v(0, 0, 0)));
  ASSERT_TRUE(check("^0", v(0, MAX, MAX, MAX)));
  ASSERT_FALSE(check("^0", v(1, 0, 0)));

  // tilde

  ASSERT_TRUE(check("~1.2.3", v(1, 2, 3)));
  ASSERT_TRUE(check("~1.2.3", v(1, 2, 3, MAX)));
  ASSERT_FALSE(check("~1.2.3", v(1, 2, 2, MAX)));
  ASSERT_FALSE(check("~1.2.3", v(1, 3, 0)));

  ASSERT_TRUE(check("~1.2", v(1, 2, 0)));
  ASSERT_TRUE(check("~1.2", v(1, 2, MAX, MAX)));
  ASSERT_FALSE(check("~1.2", v(1, 1, MAX, MAX)));
  ASSERT_FALSE(check("~1.2", v(1, 3, 0)));

  ASSERT_TRUE(check("~1", v(1, 0, 0)));
  ASSERT_TRUE(check("~1", v(1, MAX, MAX, MAX)));
  ASSERT_FALSE(check("~1", v(0, MAX, MAX, MAX)));
  ASSERT_FALSE(check("~1", v(2, 0, 0)));
}

TEST(VersioningTest, VersionConstraintsTest)
{
  // shortcut
  using v = Version;

  auto check = [](const QString& constraints, Version const& v,
                  Version::ParseMode mode = Version::ParseMode::SemVer) {
    return VersionConstraints::parse(constraints, mode).matches(v);
  };

  ASSERT_TRUE(check("2.5.2", v(2, 5, 2)));
  ASSERT_FALSE(check("2.5.3", v(2, 5, 2)));

  ASSERT_TRUE(check(">=2.5.0, <2.6.0", v(2, 5, 2)));
  ASSERT_FALSE(check(">=2.5.0, <2.6.0", v(2, 6, 0)));
  ASSERT_FALSE(check(">=2.5.0, <2.6.0", v(2, 5, 0, Development)));
  ASSERT_FALSE(check(">=2.5.0, <2.6.0", v(2, 4, 4)));
}
