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

#include "questionboxmemory.h"
#include "ui_questionboxmemory.h"

#include <QApplication>            // for QApplication
#include <QIcon>                   // for QIcon
#include <QPushButton>
#include <QMutex>                  // for QMutex
#include <QMutexLocker>
#include <QSettings>
#include <QStyle>                  // for QStyle, etc

#include <stdlib.h>                // for atexit

namespace MOBase {

QSettings *QuestionBoxMemory::s_SettingFile = nullptr;
QMutex QuestionBoxMemory::s_SettingsMutex;

QuestionBoxMemory::QuestionBoxMemory(QWidget *parent, const QString &title, const QString &text, QString const *filename,
                                     const QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton)
  : QDialog(parent)
  , ui(new Ui::QuestionBoxMemory)
  , m_Button(QDialogButtonBox::Cancel)
{
  ui->setupUi(this);

  this->setWindowFlag(Qt::WindowType::WindowContextHelpButtonHint, false);
  this->setWindowTitle(title);

  QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  ui->iconLabel->setPixmap(icon.pixmap(128));
  ui->messageLabel->setText(text);
  if (filename == nullptr) {
    //delete the 2nd check box
    QCheckBox *box = ui->rememberForCheckBox;
    box->parentWidget()->layout()->removeWidget(box);
    delete box;
  } else {
    ui->rememberForCheckBox->setText(ui->rememberForCheckBox->text() + " " + *filename);
  }
  ui->buttonBox->setStandardButtons(buttons);
  if (defaultButton != QDialogButtonBox::NoButton) {
    ui->buttonBox->button(defaultButton)->setDefault(true);
  }
  connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}


QuestionBoxMemory::~QuestionBoxMemory()
{
  delete ui;
}


void QuestionBoxMemory::init(const QString &fileName)
{
  QMutexLocker locker(&s_SettingsMutex);
  if (s_SettingFile == nullptr) {
    s_SettingFile = new QSettings(fileName, QSettings::IniFormat);
    atexit(&QuestionBoxMemory::cleanup);
  }
}

void QuestionBoxMemory::resetDialogs()
{
  s_SettingFile->remove("DialogChoices");
}

void QuestionBoxMemory::cleanup() noexcept
{
  try
  {
    QMutexLocker locker(&s_SettingsMutex);
    s_SettingFile->sync();
    delete s_SettingFile;
  } catch(...) {
    // this is called from atexit(), eat any exceptions
  }
}

void QuestionBoxMemory::buttonClicked(QAbstractButton *button)
{
  m_Button = ui->buttonBox->standardButton(button);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::query(QWidget *parent, const QString &windowName,
    const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
    QDialogButtonBox::StandardButton defaultButton)
{
  return queryImpl(parent, windowName, nullptr, title, text, buttons, defaultButton);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::query(QWidget *parent, const QString &windowName, const QString &fileName,
    const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
    QDialogButtonBox::StandardButton defaultButton)
{
  return queryImpl(parent, windowName, &fileName, title, text, buttons, defaultButton);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::queryImpl(QWidget *parent, const QString &windowName, const QString *fileName,
    const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
    QDialogButtonBox::StandardButton defaultButton)
{
  QMutexLocker locker(&s_SettingsMutex);
  QString windowSetting("DialogChoices/" + windowName);
  QString fileSetting;
  if (fileName != nullptr) {
    fileSetting = windowSetting + "/" + *fileName;
    if (s_SettingFile->contains(fileSetting)) {
      return static_cast<QDialogButtonBox::StandardButton>(s_SettingFile->value(fileSetting).toInt());
    }
  }
  if (s_SettingFile->contains(windowSetting)) {
    return static_cast<QDialogButtonBox::StandardButton>(s_SettingFile->value(windowSetting).toInt());
  }
  QuestionBoxMemory dialog(parent, title, text, fileName, buttons, defaultButton);
  dialog.exec();
  if (dialog.m_Button != QDialogButtonBox::Cancel) {
    if (dialog.ui->rememberCheckBox->isChecked()) {
      s_SettingFile->setValue(windowSetting, dialog.m_Button);
    }
    if (fileName != nullptr && dialog.ui->rememberForCheckBox->isChecked()) {
      s_SettingFile->setValue(fileSetting, dialog.m_Button);
    }
  }
  return dialog.m_Button;
}

}
