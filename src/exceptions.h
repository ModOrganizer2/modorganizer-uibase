#ifndef UIBASE_EXCEPTIONS_H
#define UIBASE_EXCEPTIONS_H

#include <stdexcept>

#include <QString>

#include "dllimport.h"

namespace MOBase {

#pragma warning(push)
#pragma warning(disable: 4275)  // non-dll interface

/**
  * @brief exception class that takes a QString as the parameter
  **/
class QDLLEXPORT MOException : public std::exception {
public:

  MOException(const QString& text) : m_Message(text.toUtf8()) { }

  virtual const char* what() const noexcept override {
    return m_Message.constData();
  }

private:
  QByteArray m_Message;
};

#pragma warning(pop)


// Exception thrown in case of incompatibilities, i.e. between plugins.
class QDLLEXPORT IncompatibilityException : public MOException {
public:
  using MOException::MOException;
};

// Exception thrown for invalid NXM links.
class QDLLEXPORT InvalidNXMLinkException : public MOException {
public:
  InvalidNXMLinkException(const QString& link) :
    MOException(QObject::tr("invalid nxm-link: %1").arg(link)) { }
};

// alias for backward-compatibility, should be removed when possible
using MyException = MOException;

}

#endif