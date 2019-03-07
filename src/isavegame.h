#ifndef ISAVEGAMEINFO_H
#define ISAVEGAMEINFO_H

#include <QMetaType>

class QString;
class QDateTime;

namespace MOBase {

/** Base class for information about what is in a save game */
class ISaveGame
{
public:
  virtual ~ISaveGame() {}

  /**
   * @brief get the name of the (main) save file.
   */
  virtual QString getFilename() const = 0;

  /**
   * @brief get the creation time of the save.
   *
   * Note that this might not be the same as the creation time of the file.
   */
  virtual QDateTime getCreationTime() const = 0;

  /**
   * @brief get a name which can be used to identify sets of saves to transfer
   * between profiles.
   *
   * This is normally the pc name for RPG games.
   */
  virtual QString getSaveGroupIdentifier() const = 0;

  /**
   * @brief Gets all the files related to this save
   *
   * Note: This must return the actual list, not the potential list.
   */
  virtual QStringList allFiles() const = 0;

  /**
  * @brief See if the save file has an associated script extender save
  */
  virtual bool hasScriptExtenderFile() const = 0;
};

}

Q_DECLARE_METATYPE(MOBase::ISaveGame const *)

#endif // SAVEGAMEINFO_H
