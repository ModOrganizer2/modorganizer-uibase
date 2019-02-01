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
#include <QRegularExpression>
#include <QStringList>

NXMUrl::NXMUrl(const QString &url)
{
	QRegularExpression exp("nxm://([a-z0-9]+)/mods/(\\d+)/files/(\\d+)\\?key\\=([^&]+)\\&expires\\=(\\d+)", QRegularExpression::CaseInsensitiveOption);
	auto match = exp.match(url, 0, QRegularExpression::PartialPreferFirstMatch);
	//QString pattern = QRegularExpression::escape("key=") + "([a-z0-9]+)" + QRegularExpression::escape("&expires=") + "(\\d+)";
	//QRegularExpression exp2(pattern, QRegularExpression::CaseInsensitiveOption);
	//auto match2 = exp2.match(url, 0, QRegularExpression::PartialPreferFirstMatch);
	if (exp.captureCount() < 3) {
		throw MOBase::MyException(tr("invalid nxm-link: %1").arg(url));
	}
	m_Game = match.captured(1);
	m_ModId = match.captured(2).toInt();
	m_FileId = match.captured(3).toInt();
	m_Key = match.captured(4);
	m_Expires = match.captured(5).toInt();
}
