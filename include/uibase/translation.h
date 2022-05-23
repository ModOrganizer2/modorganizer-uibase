#ifndef UIBASE_TRANSLATION_H
#define UIBASE_TRANSLATION_H

#include <filesystem>
#include <string>

#include "dllimport.h"

namespace MOBase
{

// class representing a base translation for MO2
//
class QDLLEXPORT Translation
{
  std::string identifier_, language_;
  std::vector<std::filesystem::path> qm_files_;

public:
  Translation(std::string_view identifier, std::string_view language,
              std::vector<std::filesystem::path> qm_files)
      : identifier_{identifier}, language_{language}, qm_files_{std::move(qm_files)}
  {}

  // retrieve the identifier of the translation, e.g., en or fr_FR
  //
  const auto& identifier() const { return identifier_; }

  // retrieve the language of this translation
  //
  const auto& language() const { return language_; }

  // retrieve the path to the QM files including with this translation
  //
  const auto& files() const { return qm_files_; }
};

// class representing the extension of a base translation
//
class QDLLEXPORT TranslationAddition
{
  std::string baseIdentifier_;
  std::vector<std::filesystem::path> qm_files_;

public:
  TranslationAddition(std::string_view baseIdentifier,
                      std::vector<std::filesystem::path> qm_files)
      : baseIdentifier_{baseIdentifier}, qm_files_{std::move(qm_files)}
  {}

  // retrieve the identifier of the base translation, if there is one
  //
  const auto& baseIdentifier() const { return baseIdentifier_; }

  // retrieve the path to the stylesheet for this extension
  //
  const auto& files() const { return qm_files_; }
};

}  // namespace MOBase

#endif
