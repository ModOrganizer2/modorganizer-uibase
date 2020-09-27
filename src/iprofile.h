#ifndef IPROFILE
#define IPROFILE


#include <QString>

namespace MOBase {


class IProfile {

public:

  virtual ~IProfile() {}

  virtual QString name() const = 0;
  virtual QString absolutePath() const = 0;
  virtual bool localSavesEnabled() const = 0;
  virtual bool localSettingsEnabled() const = 0;
  virtual bool invalidationActive(bool *supported) const = 0;

  /**
   * @brief Retrieve the absolute file to the corresponding file.
   *
   * @param iniFile INI file to retrieve a path for. This can either be the
   *     name of a file or a path to the absolute file outside of the profile.
   *
   * @return the absolute path for the given INI file for this profile.
   *
   * @note If iniFile does not correspond to a file in the list of ini files for the 
   *     current game (as returned by IPluginGame::iniFiles), the path to the global 
   *     file will be returned (if iniFile is absolute, iniFile is returned, otherwiise 
         the path is assumed relative to the game documents directory).
   */
  virtual QString iniFilePath(QString iniFile) const = 0;
};

} // namespace MOBase


#endif // IPROFILE
