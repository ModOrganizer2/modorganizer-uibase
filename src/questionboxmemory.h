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

#ifndef QUESTIONBOXMEMORY_H
#define QUESTIONBOXMEMORY_H

#include "dllimport.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QObject>
#include <QString>

class QAbstractButton;
class QMutex;
class QSettings;
class QWidget;

namespace Ui {
  class QuestionBoxMemory;
}


namespace MOBase {


class QDLLEXPORT QuestionBoxMemory : public QDialog
{
  Q_OBJECT

public:

  virtual ~QuestionBoxMemory();

  static void init(const QString &fileName);

  static void resetDialogs();

  static QDialogButtonBox::StandardButton query(QWidget *parent, const QString &windowName,
                                                const QString &title, const QString &text,
                                                QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes | QDialogButtonBox::No,
                                                QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);

  static QDialogButtonBox::StandardButton query(QWidget *parent, const QString &windowName, const QString &fileName,
                                                const QString &title, const QString &text,
                                                QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes | QDialogButtonBox::No,
                                                QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);

private slots:

  void buttonClicked(QAbstractButton *button);

private:

  explicit QuestionBoxMemory(QWidget *parent, const QString &title, const QString &text, const QString *filename, const QDialogButtonBox::StandardButtons buttons,
                             QDialogButtonBox::StandardButton defaultButton);

  static void cleanup() noexcept;

private:

  static QMutex s_SettingsMutex;
  static QSettings *s_SettingFile;

  static QDialogButtonBox::StandardButton queryImpl(QWidget *parent, const QString &windowName, const QString *fileName,
                                                const QString &title, const QString &text,
                                                QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Yes | QDialogButtonBox::No,
                                                QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::NoButton);

  Ui::QuestionBoxMemory *ui;
  QDialogButtonBox::StandardButton m_Button;

};

}

#endif // QUESTIONBOXMEMORY_H
