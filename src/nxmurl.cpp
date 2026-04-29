/*
Copyright (C) 2012 Sebastian Herbord. All rights reserved.

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
  // nxm://starfield/collections/7ou329/revisions/24
  QRegularExpression modsExp("nxm://[a-z0-9]+/mods/(\\d+)/files/(\\d+)",
                             QRegularExpression::CaseInsensitiveOption);
  QRegularExpression collectionsExp(
      "nxm://[a-z0-9]+/collections/([a-z0-9]+)/revisions/(\\d+)",
      QRegularExpression::CaseInsensitiveOption);
  auto match           = modsExp.match(url);
  bool collectionsLink = false;
  if (!match.hasMatch()) {
    match = collectionsExp.match(url);
    if (!match.hasMatch()) {
      throw MOBase::InvalidNXMLinkException(url);
    } else {
      collectionsLink = true;
    }
  }
  m_Game = nxm.host();
  if (!collectionsLink) {
    m_ModId      = match.captured(1).toInt();
    m_FileId     = match.captured(2).toInt();
    m_Key        = query.queryItemValue("key");
    m_Expires    = query.queryItemValue("expires").toInt();
    m_UserId     = query.queryItemValue("user_id").toInt();
    m_Collection = false;
  } else {
    m_CollectionId       = match.captured(1);
    m_CollectionRevision = match.captured(2).toInt();
    m_Collection         = true;
  }
}
