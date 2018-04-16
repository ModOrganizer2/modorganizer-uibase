#ifndef MODREPOSITORYFILEINFO_H
#define MODREPOSITORYFILEINFO_H

#include "versioninfo.h"
#include <QString>
#include <QDateTime>
#include <QVariantMap>

namespace MOBase {
  enum EFileCategory {
    TYPE_UNKNOWN = 0,
    TYPE_MAIN,
    TYPE_UPDATE,
    TYPE_OPTION
  };

  class QDLLEXPORT ModRepositoryFileInfo : public QObject {
    Q_OBJECT

  public:

    ModRepositoryFileInfo(const ModRepositoryFileInfo &reference);
    ModRepositoryFileInfo(QString gameName = "", int modID = 0, int fileID = 0);
    ModRepositoryFileInfo(const QString &data);
    QString toString() const;

    QString name;
    QString uri;
    QString description;
    VersionInfo version;
    VersionInfo newestVersion;
    int categoryID;
    QString modName;
    QString gameName;
    int modID;
    int fileID;
    size_t fileSize;
    QString fileName;
    int fileCategory;
    QDateTime fileTime;
    QString repository;

    QVariantMap userData;
  };
}

#endif // MODREPOSITORYFILEINFO_H
