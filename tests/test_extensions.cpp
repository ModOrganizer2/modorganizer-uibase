#pragma warning(push)
#pragma warning(disable : 4668)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <QJsonDocument>

#include <uibase/extensions/extension.h>

#include <format>

using namespace MOBase;

class TestMetaData : public ExtensionMetaData
{
public:
  TestMetaData(std::filesystem::path const& path, QByteArray const& metadata)
      : ExtensionMetaData(path, QJsonDocument::fromJson(metadata).object())
  {}
};

TEST(ExtensionsTest, MetaData)
{
  const auto metadata = TestMetaData({}, R"({
  "id": "mo2-game-bethesda",
  "name": "Elder Scrolls & Fallout Games",
  "version": "1.0.0",
  "description": "ModOrganizer2 support for The Elder Scrolls & Fallout games.",
  "author": {
    "name": "Mod Organizer 2",
    "homepage": "https://www.modorganizer.org/"
  },
  "icon": "./tests/icon.png",
  "contributors": [
    "AL",
    "AnyOldName3",
    "Holt59",
    "Silarn"
  ],
  "type": "game",
  "content": {
    "plugins": {
      "autodetect": true
    },
    "translations": {
      "autodetect": "translations"
    }
  }
})");

  EXPECT_EQ("mo2-game-bethesda", metadata.identifier());
  EXPECT_EQ("Elder Scrolls & Fallout Games", metadata.name());
  EXPECT_FALSE(metadata.icon().isNull());
}
