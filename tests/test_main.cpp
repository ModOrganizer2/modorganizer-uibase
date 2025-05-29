#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QTranslator>

#include <uibase/log.h>

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);

  MOBase::log::createDefault({.name     = "./mo2-tests.logs",
                              .maxLevel = MOBase::log::Levels::Info,
                              .pattern  = ""});

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
