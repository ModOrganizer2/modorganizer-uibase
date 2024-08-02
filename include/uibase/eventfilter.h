/*
Copyright (C) 2016 Sebastian Herbord. All rights reserved.

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

#pragma once

#include "dllimport.h"
#include <QObject>
#include <functional>

namespace MOBase
{

class QDLLEXPORT EventFilter : public QObject
{

  Q_OBJECT

  typedef std::function<bool(QObject*, QEvent*)> HandlerFunc;

public:
  EventFilter(QObject* parent, const HandlerFunc& handler);

  virtual bool eventFilter(QObject* obj, QEvent* event) override;

private:
  HandlerFunc m_Handler;
};

}  // namespace MOBase
