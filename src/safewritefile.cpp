/*
Copyright (C) 2014 Sebastian Herbord. All rights reserved.

This file is part of Mod Organizer.

Mod Organizer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Mod Organizer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Mod Organizer.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "safewritefile.h"
#include "log.h"
#include <QCryptographicHash>
#include <QList>
#include <QStorageInfo>
#include <QString>

namespace MOBase
{

SafeWriteFile::SafeWriteFile(const QString& fileName) : m_FileName(fileName)
{
  if (!m_TempFile.open()) {
    const auto av =
        static_cast<double>(QStorageInfo(QDir::tempPath()).bytesAvailable());

    log::error("failed to create temporary file for '{}', error {} ('{}'), "
               "temp path is '{}', {:.3f}GB available",
               m_FileName, m_TempFile.error(), m_TempFile.errorString(),
               QDir::tempPath(), (av / 1024 / 1024 / 1024));

    QString errorMsg =
        QObject::tr(
            "Failed to save '%1', could not create a temporary file: %2 (error %3)")
            .arg(m_FileName)
            .arg(m_TempFile.errorString())
            .arg(m_TempFile.error());

    throw Exception(errorMsg);
  }
}

QFile* SafeWriteFile::operator->()
{
  Q_ASSERT(m_TempFile.isOpen());
  return &m_TempFile;
}

void SafeWriteFile::commit()
{
  shellDeleteQuiet(m_FileName);
  m_TempFile.rename(m_FileName);
  m_TempFile.setAutoRemove(false);
  m_TempFile.close();
}

bool SafeWriteFile::commitIfDifferent(QByteArray& inHash)
{
  QByteArray newHash = hash();
  if (newHash != inHash || !QFile::exists(m_FileName)) {
    commit();
    inHash = newHash;
    return true;
  } else {
    return false;
  }
}

QByteArray SafeWriteFile::hash()
{

  qint64 pos = m_TempFile.pos();
  m_TempFile.seek(0);
  QByteArray data = m_TempFile.readAll();
  m_TempFile.seek(pos);
  return QCryptographicHash::hash(data, QCryptographicHash::Md5);
}

}  // namespace MOBase
