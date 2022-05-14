#ifndef UNMANAGEDMODS_H
#define UNMANAGEDMODS_H

#include <QFileInfo>
#include <QString>
#include <QStringList>

class UnmanagedMods
{
public:
  /**
   * @param onlyOfficial if set, only official mods (dlcs) are returned
   *
   * @return the list of unmanaged mods (internal names)
   */
  virtual QStringList mods(bool onlyOfficial) const = 0;

  /**
   * @param modName (internal) name of the mod being requested
   * @return display name of the mod
   */
  virtual QString displayName(const QString& modName) const = 0;

  /**
   * @param modName name of the mod being requested
   * @return reference file info
   */
  virtual QFileInfo referenceFile(const QString& modName) const = 0;

  /**
   * @param modName name of the mod being requested
   * @return list of file names (absolute paths)
   */
  virtual QStringList secondaryFiles(const QString& modName) const = 0;

  virtual ~UnmanagedMods() {}
};

#endif  // UNMANAGEDMODS_H
