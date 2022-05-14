/*
Mod Organizer shared UI functionality

Copyright (C) 2012 Sebastian Herbord, 2022 MO2 Team. All rights reserved.

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

#include "nxmurl.h"
#include "utility.h"
#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QUrl>
#include <QUrlQuery>

NXMUrl::NXMUrl(const QString& url)
{
  QUrl nxm(url);
  QUrlQuery query(nxm);
  QRegularExpression exp("nxm://[a-z0-9]+/mods/(\\d+)/files/(\\d+)",
                         QRegularExpression::CaseInsensitiveOption);
  auto match = exp.match(url);
  if (!match.hasMatch()) {
    throw MOBase::InvalidNXMLinkException(url);
  }
  m_Game    = nxm.host();
  m_ModId   = match.captured(1).toInt();
  m_FileId  = match.captured(2).toInt();
  m_Key     = query.queryItemValue("key");
  m_Expires = query.queryItemValue("expires").toInt();
  m_UserId  = query.queryItemValue("user_id").toInt();
}
