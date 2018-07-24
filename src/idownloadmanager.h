#ifndef IDOWNLOADMANAGER_H
#define IDOWNLOADMANAGER_H


#include "dllimport.h"
#include <QStringList>
#include <QObject>


namespace MOBase {

class QDLLEXPORT IDownloadManager : public QObject
{
  Q_OBJECT

public:

  IDownloadManager(QObject *parent = nullptr)
    : QObject(parent)
  {}

  /**
   * @brief download a file by url. The list can contain alternative URLs to allow the
   *        download manager to switch in case of download problems
   * @param urls list of urls to download from
   * @return an id by which the download will be identified
   */
  virtual int startDownloadURLs(const QStringList &urls) = 0;

  /**
   * @brief download a file from www.nexusmods.com/<game>. <game> is always the game currently being managed
   * @param modID id of the mod for which to download a file
   * @param fileID id of the file to download
   * @return an id by which the download will be identified
   */
  virtual int startDownloadNexusFile(int modID, int fileID) = 0;

  /**
   * @brief get the (absolute) file path of the specified download.
   * @param id id of the download as returned by the download... functions
   * @return absoute path to the downloaded file. This file may not yet exist if the download is incomplete
   */
  virtual QString downloadPath(int id) = 0;

Q_SIGNALS:

  /**
   * @brief emitted when the download has finished completely (including retrieval of meta information)
   * @param id id of the download
   */
  void downloadComplete(int id);

  /**
   * @brief emitted when the download has been paused by the user
   * @param id id of the download
   */
  void downloadPaused(int id);

  /**
   * @brief emitted when the download has failed
   * @param id id of the download
   */
  void downloadFailed(int id);

  /**
   * @brief emitted when the download has been removed from the download manager.
   *        The id of the download is no longer valid
   * @param id id of the download
   */
  void downloadRemoved(int id);

};

}

#endif // IDOWNLOADMANAGER_H
