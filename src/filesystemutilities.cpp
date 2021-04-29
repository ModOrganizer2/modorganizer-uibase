#include "filesystemutilities.h"

#include <QString>
#include <QRegularExpression>

namespace MOBase
{

QString sanitizeFileName(const QString& name, const QString& replacement)
{
  QString new_name = name;

  // Remove characters not allowed by Windows
  new_name.replace(QRegularExpression("[\\x{00}-\\x{1f}\\\\/:\\*\\?\"<>|]"), replacement);

  // Don't end with a period or a space
  // Don't be "." or ".."
  new_name.remove(QRegularExpression("[\\. ]*$"));

  // Recurse until stuff stops changing
  if (new_name != name) {
    return sanitizeFileName(new_name);
  }
  return new_name;
}

bool validFileName(const QString& name)
{
  if (name.isEmpty()) {
    return false;
  }
  if (name == "." || name == "..")
  {
    return false;
  }

  return (name == sanitizeFileName(name));
}

} // namespace MOBase
