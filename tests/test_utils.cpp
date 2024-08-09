#include "test_utils.h"

#include <QCoreApplication>

namespace mo2::tests
{
TranslationHelper::TranslationHelper() {}

TranslationHelper::~TranslationHelper()
{
  release();
}

void TranslationHelper::release()
{
  if (m_Translator) {
    QCoreApplication::removeTranslator(m_Translator.get());
    m_Translator.reset();
  }
}

void TranslationHelper::switchLanguage(const QString& lang)
{
  m_Translator = std::make_unique<QTranslator>();
  if (m_Translator->load("tests_" + lang, "tests/translations")) {
    QCoreApplication::installTranslator(m_Translator.get());
  } else {
    m_Translator.reset();
  }
}
}  // namespace mo2::tests
