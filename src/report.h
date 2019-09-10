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

#ifndef REPORT_H
#define REPORT_H

#include "dllimport.h"
#include <QString>
#include <QMessageBox>
#include <wchar.h>

namespace MOBase {

/**
 * Convenience function displaying an error message box. This function uses WinAPI if no Qt Window is available
 * yet or QMessageBox otherwise.
 */
QDLLEXPORT void reportError(const QString &message);


struct QDLLEXPORT TaskDialogButton
{
  QString text, description;
  QMessageBox::StandardButton button;

  TaskDialogButton(QString text, QString description, QMessageBox::StandardButton button);
  TaskDialogButton(QString text, QMessageBox::StandardButton button);
};

QDLLEXPORT QMessageBox::StandardButton taskDialog(
  QWidget* parent, const QString& title,
  const QString& mainText, const QString& content, const QString& details,
  std::vector<TaskDialogButton> buttons={});

} // namespace MOBase

#endif // REPORT_H
