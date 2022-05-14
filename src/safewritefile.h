/*
Mod Organizer shared UI functionality

Copyright (C) 2012 Sebastian Herbord. All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef SAFEWRITEFILE_H
#define SAFEWRITEFILE_H

#include <QList>
#include <QString>
#include <QTemporaryFile>

#include "dllimport.h"
#include "utility.h"

namespace MOBase
{

/**
 * @brief a wrapper for QFile that ensures the file is only actually (over-)written
 * if writing was successful
 */
class QDLLEXPORT SafeWriteFile
{
public:
  SafeWriteFile(const QString& fileName);

  QFile* operator->();

  void commit();

  bool commitIfDifferent(QByteArray& hash);

private:
  QByteArray hash();

private:
  QString m_FileName;
  QTemporaryFile m_TempFile;
};

}  // namespace MOBase

#endif  // SAFEWRITEFILE_H
