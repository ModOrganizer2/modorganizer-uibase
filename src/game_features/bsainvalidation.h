#ifndef BSAINVALIDATION_H
#define BSAINVALIDATION_H

namespace MOBase
{
class IProfile;
}

class QString;

class BSAInvalidation
{
public:
  virtual bool isInvalidationBSA(const QString& bsaName) = 0;
  virtual void deactivate(MOBase::IProfile* profile)     = 0;
  virtual void activate(MOBase::IProfile* profile)       = 0;
  virtual bool prepareProfile(MOBase::IProfile* profile) = 0;

  virtual ~BSAInvalidation() {}
};

#endif  // BSAINVALIDATION_H
