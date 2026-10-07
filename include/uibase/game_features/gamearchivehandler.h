#ifndef UIBASE_GAMEFEATURES_GAMEARCHIVEHANDLER_H
#define UIBASE_GAMEFEATURES_GAMEARCHIVEHANDLER_H

#include <functional>

#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "./game_feature.h"

namespace MOBase
{

/**
 * @brief Generic archive handler for game-specific archive formats.
 *
 * This feature exists so tools can delegate archive handling to the active game plugin
 * when the archive format is game-specific. Bethesda archives remain handled through
 * bsatk; non-Bethesda archive ownership belongs in game plugins rather than in uibase
 * or bsatk.
 */
class GameArchiveHandler : public details::GameFeatureCRTP<GameArchiveHandler>
{
public:
  using ProgressCallback = std::function<void(qint64 current, qint64 total)>;

  /**
   * @brief Check whether this handler can process the archive at the given path.
   */
  virtual bool supportsArchive(const QString& archivePath) const = 0;

  /**
   * @brief Extract an archive into the given output directory.
   *
   * @return true on success, false on failure or cancellation.
   */
  virtual bool extractArchive(const QString& archivePath,
                              const QString& outputDirectory,
                              const ProgressCallback& progress = {},
                              QString* errorMessage            = nullptr) const = 0;

  /**
   * @brief File dialog name filters for archives this handler can process.
   *
   * NOTE: Kept after legacy virtuals to preserve ABI slot order for
   * supportsArchive()/extractArchive() with pre-existing MO2 binaries.
   */
  virtual QStringList supportedArchiveNameFilters() const { return {}; }

  /**
   * @brief Check whether this handler can create the given archive.
   */
  virtual bool canCreateArchive(const QString&) const { return false; }

  /**
   * @brief Create an archive from the given source directory.
   *
   * @return true on success, false if archive creation is unsupported, fails, or is
   * cancelled.
   */
  virtual bool createArchive(const QString&, const QString&,
                             const ProgressCallback& = {}, QString* = nullptr) const
  {
    return false;
  }
};

}  // namespace MOBase

#endif
