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
   */
  virtual QString iniFilePath(QString iniFile) = 0;
};

} // namespace MOBase


#endif // IPROFILE
