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
#include "log.h"

#include <QApplication>
#include <QIcon>
#include <QPushButton>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>
#include <QStyle>

namespace MOBase
{

static QMutex g_mutex;
static QuestionBoxMemory::GetButton g_get;
static QuestionBoxMemory::SetWindowButton g_setWindow;
static QuestionBoxMemory::SetFileButton g_setFile;

QString buttonToString(QDialogButtonBox::StandardButton b)
{
  if (b == QDialogButtonBox::Yes) {
    return QString("'yes' (0x%1)").arg(static_cast<int>(b), 0, 16);
  } else if (b == QDialogButtonBox::No) {
    return QString("'no' (0x%1)").arg(static_cast<int>(b), 0, 16);
  } else {
    return QString("0x%1").arg(static_cast<int>(b), 0, 16);
  }
}


QuestionBoxMemory::QuestionBoxMemory(
  QWidget *parent, const QString &title, const QString &text, QString const *filename,
  const QDialogButtonBox::StandardButtons buttons, QDialogButtonBox::StandardButton defaultButton)
    : QDialog(parent)
    , ui(new Ui::QuestionBoxMemory)
    , m_Button(QDialogButtonBox::Cancel)
{
  ui->setupUi(this);

  setWindowFlag(Qt::WindowType::WindowContextHelpButtonHint, false);
  setWindowTitle(title);

  QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  ui->iconLabel->setPixmap(icon.pixmap(128));
  ui->messageLabel->setText(text);

  if (filename == nullptr) {
    //delete the 2nd check box
    QCheckBox *box = ui->rememberForCheckBox;
    box->parentWidget()->layout()->removeWidget(box);
    delete box;
  } else {
    ui->rememberForCheckBox->setText(
      ui->rememberForCheckBox->text().arg(*filename));
  }

  ui->buttonBox->setStandardButtons(buttons);

  if (defaultButton != QDialogButtonBox::NoButton) {
    ui->buttonBox->button(defaultButton)->setDefault(true);
  }

  connect(
    ui->buttonBox, SIGNAL(clicked(QAbstractButton*)),
    this, SLOT(buttonClicked(QAbstractButton*)));
}

QuestionBoxMemory::~QuestionBoxMemory() = default;

void QuestionBoxMemory::setCallbacks(
  GetButton get, SetWindowButton setWindow, SetFileButton setFile)
{
  QMutexLocker locker(&g_mutex);

  g_get = get;
  g_setWindow = setWindow;
  g_setFile = setFile;
}

void QuestionBoxMemory::buttonClicked(QAbstractButton *button)
{
  m_Button = ui->buttonBox->standardButton(button);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::query(
  QWidget *parent, const QString &windowName,
  const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
  QDialogButtonBox::StandardButton defaultButton)
{
  return queryImpl(parent, windowName, nullptr, title, text, buttons, defaultButton);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::query(
  QWidget *parent, const QString &windowName, const QString &fileName,
  const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
  QDialogButtonBox::StandardButton defaultButton)
{
  return queryImpl(parent, windowName, &fileName, title, text, buttons, defaultButton);
}

QDialogButtonBox::StandardButton QuestionBoxMemory::queryImpl(
  QWidget *parent, const QString &windowName, const QString *fileName,
  const QString &title, const QString &text, QDialogButtonBox::StandardButtons buttons,
  QDialogButtonBox::StandardButton defaultButton)
{
  QMutexLocker locker(&g_mutex);

  const auto button = g_get(windowName, (fileName ? *fileName : ""));
  if (button != NoButton) {
    log::debug(
      "{}: not asking because user always wants response {}",
      windowName + (fileName ? QString("/") + fileName : ""),
      buttonToString(button));

    return button;
  }

  QuestionBoxMemory dialog(parent, title, text, fileName, buttons, defaultButton);
  dialog.exec();

  if (dialog.m_Button != QDialogButtonBox::Cancel) {
    if (dialog.ui->rememberCheckBox->isChecked()) {
      log::debug(
        "remembering choice {} for window {}",
        buttonToString(dialog.m_Button), windowName);

      g_setWindow(windowName, dialog.m_Button);
    }

    if (fileName != nullptr && dialog.ui->rememberForCheckBox->isChecked()) {
      log::debug(
        "remembering choice {} for file {}",
        buttonToString(dialog.m_Button), windowName + "/" + *fileName);

      g_setFile(windowName, *fileName, dialog.m_Button);
    }
  }

  return dialog.m_Button;
}

} // namespace
