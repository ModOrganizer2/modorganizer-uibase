#pragma once

#include <memory>

#include <QString>
#include <QTranslator>

namespace mo2::tests
{

class TranslationHelper
{
public:
  // create a new translation helper that can be used to switch language during tests
  TranslationHelper();
  ~TranslationHelper();

  // switch to the given language (should be available)
  void switchLanguage(const QString& lang);

private:
  std::unique_ptr<QTranslator> m_Translator;

  void release();
};

}  // namespace mo2::tests
