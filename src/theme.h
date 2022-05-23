#ifndef UIBASE_THEME_H
#define UIBASE_THEME_H

#include <filesystem>
#include <string>

#include <QRegularExpression>

#include "dllimport.h"

namespace MOBase
{

// class representing a base theme for MO2, e.g., VS Dark or Skyrim
//
class QDLLEXPORT Theme
{
  std::string identifier_, name_;
  std::filesystem::path stylesheet_;

public:
  Theme(std::string_view identifier, std::string_view name,
        std::filesystem::path stylesheet)
      : identifier_{identifier}, name_{name}, stylesheet_{std::move(stylesheet)}
  {}

  // retrieve the identifier of the theme
  //
  const auto& identifier() const { return identifier_; }

  // retrieve the name of the theme
  //
  const auto& name() const { return name_; }

  // retrieve the path to the stylesheet of the theme
  //
  const auto& stylesheet() const { return stylesheet_; }
};

// class representing additions for a base theme
//
class QDLLEXPORT ThemeAddition
{
  QRegularExpression baseThemeExpr_;
  std::filesystem::path stylesheet_;

public:
  ThemeAddition(std::filesystem::path stylesheet)
      : ThemeAddition{"*", std::move(stylesheet)}
  {}

  ThemeAddition(std::string_view baseIdentifier, std::filesystem::path stylesheet);

  // retrieve the identifier of the base theme, if there is one
  //
  bool isAdditionFor(Theme const& theme) const;

  // retrieve the path to the stylesheet for this extension
  //
  const auto& stylesheet() const { return stylesheet_; }
};

}  // namespace MOBase

#endif
