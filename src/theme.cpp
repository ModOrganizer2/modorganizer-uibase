#include "extensions/theme.h"

#include "utility.h"

namespace MOBase
{

ThemeAddition::ThemeAddition(std::string_view baseIdentifier,
                             std::filesystem::path stylesheet)
    : baseThemeExpr_{QRegularExpression::fromWildcard(
          ToQString(baseIdentifier), Qt::CaseInsensitive,
          QRegularExpression::DefaultWildcardConversion)},
      stylesheet_{std::move(stylesheet)}
{}

bool ThemeAddition::isAdditionFor(Theme const& theme) const
{
  return baseThemeExpr_.match(ToQString(theme.identifier())).hasMatch();
}

}  // namespace MOBase
