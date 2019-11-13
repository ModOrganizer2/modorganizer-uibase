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
#include <QStringList>
#include <QCryptographicHash>


namespace MOBase {


SafeWriteFile::SafeWriteFile(const QString &fileName)
: m_FileName(fileName)
{
  if (!m_TempFile.open()) {
    log::error(
      "attempt to create file at {} for {} failed (error {}: {})"
      , QDir::tempPath()
      , m_FileName
      , m_TempFile.error()
      , m_TempFile.errorString());
    
    QString errorMsg;
    switch (m_TempFile.error()) {
      case QFileDevice::FileError::PermissionsError:
        errorMsg = QObject::tr(
          "not allowed to access directory %1. \
           Make sure the permissions on the directory allow reading and writing")
          .arg(QDir::tempPath());
        break;
      case QFileDevice::FileError::ResourceError:
        errorMsg = QObject::tr(
          "not enough resources to create a file in %1. Make sure there is \
           enough space left on the device")
          .arg(QDir::tempPath());
        break;
      case QFileDevice::FileError::NoError:  // fall-through
      case QFileDevice::FileError::WriteError:
      case QFileDevice::FileError::FatalError:
      case QFileDevice::FileError::OpenError:
      case QFileDevice::FileError::AbortError:
      case QFileDevice::FileError::TimeOutError:
      case QFileDevice::FileError::UnspecifiedError:
      case QFileDevice::FileError::RemoveError:
      case QFileDevice::FileError::RenameError:
      case QFileDevice::FileError::PositionError:
      case QFileDevice::FileError::ResizeError:
      case QFileDevice::FileError::CopyError:
      default:
        errorMsg = QObject::tr("failed to open temporary file: %1").arg(m_TempFile.errorString());
    }

    throw MyException(errorMsg);
  }
}


QFile *SafeWriteFile::operator->() {
  Q_ASSERT(m_TempFile.isOpen());
  return &m_TempFile;
}


void SafeWriteFile::commit() {
  shellDeleteQuiet(m_FileName);
  m_TempFile.rename(m_FileName);
  m_TempFile.setAutoRemove(false);
  m_TempFile.close();
}

bool SafeWriteFile::commitIfDifferent(QByteArray &inHash) {
  QByteArray newHash = hash();
  if (newHash != inHash
      || !QFile::exists(m_FileName)) {
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

}
