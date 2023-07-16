#ifndef UIBASE_IEXTENSIONLIST_H
#define UIBASE_IEXTENSIONLIST_H

#include <cstdlib>

#include <QString>

namespace MOBase
{

class IExtension;

// interface to the list of extensions in MO2
//
class IExtensionList
{
public:
  // check if the extension with the given identifier is installed or not
  //
  virtual bool installed(const QString& identifier) const = 0;

  // check if the extension with the given identifier is installed and enabled
  //
  virtual bool enabled(const QString& extension) const = 0;

  // check if the extension is enabled or not
  //
  virtual bool enabled(const IExtension& extension) const = 0;

  // retrieve the extension with the given identifier, throw std::out_of_range if no
  // such extension is installed
  //
  virtual const IExtension& get(QString const& identifier) const = 0;

  // retrieve the installed extension at the given index, throw std::out_of_range if the
  // index is out of range
  //
  virtual const IExtension& at(std::size_t const& index) const         = 0;
  virtual const IExtension& operator[](std::size_t const& index) const = 0;

  // retrieve the number of installed extensions
  //
  virtual std::size_t size() const = 0;

public:
  virtual ~IExtensionList() {}
};

}  // namespace MOBase

#endif
