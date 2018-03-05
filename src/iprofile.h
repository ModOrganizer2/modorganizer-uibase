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

};

} // namespace MOBase


#endif // IPROFILE

