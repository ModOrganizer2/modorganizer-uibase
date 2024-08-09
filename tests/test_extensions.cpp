#pragma warning(push)
#pragma warning(disable : 4668)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <QJsonDocument>

#include <uibase/extensions/extension.h>

#include <format>

#include "test_utils.h"

using namespace MOBase;

TEST(ExtensionsTest, MetaData)
{
  mo2::tests::TranslationHelper tr;

  const auto metadata = ExtensionFactory::loadMetaData(
      "./tests/data/extensions/mo2-example-extension/metadata.json");

  tr.switchLanguage("en");
  EXPECT_EQ("mo2-example-extension", metadata.identifier());
  EXPECT_EQ("Example Extension for UI Base Tests", metadata.name());

  tr.switchLanguage("fr");
  EXPECT_EQ("mo2-example-extension", metadata.identifier());
  EXPECT_EQ("Extension Démo pour les tests UI Base", metadata.name());

  EXPECT_FALSE(metadata.icon().isNull());
}
