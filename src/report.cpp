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


#include "report.h"
#include "utility.h"
#include "expanderwidget.h"
#include "ui_taskdialog.h"
#include "log.h"
#include <QApplication>
#include <QLabel>
#include <QCommandLinkButton>
#include <QComboBox>
#include <Windows.h>

namespace MOBase
{

void reportError(const QString &message)
{
  if (QApplication::topLevelWidgets().count() != 0) {
    QMessageBox messageBox(QMessageBox::Warning, QObject::tr("Error"), message, QMessageBox::Ok);
    messageBox.exec();
  } else {
    ::MessageBoxW(nullptr, message.toStdWString().c_str(), QObject::tr("Error").toStdWString().c_str(), MB_ICONERROR | MB_OK);
  }
}


TaskDialogButton::TaskDialogButton(QString t, QString d, QMessageBox::StandardButton b)
  : text(std::move(t)), description(std::move(d)), button(b)
{
}

TaskDialogButton::TaskDialogButton(QString t, QMessageBox::StandardButton b)
  : TaskDialogButton(std::move(t), {}, b)
{
}


TaskDialog::TaskDialog(QWidget* parent, QString title) :
  m_dialog(new QDialog(parent)), ui(new Ui::TaskDialog),
  m_title(std::move(title)), m_icon(QMessageBox::NoIcon),
  m_result(QMessageBox::Cancel)
{
  ui->setupUi(m_dialog.get());
}

TaskDialog::~TaskDialog() = default;

TaskDialog& TaskDialog::title(const QString& s)
{
  m_title = s;
  return *this;
}

TaskDialog& TaskDialog::main(const QString& s)
{
  m_main = s;
  return *this;
}

TaskDialog& TaskDialog::content(const QString& s)
{
  m_content = s;
  return *this;
}

TaskDialog& TaskDialog::details(const QString& s)
{
  m_details = s;
  return *this;
}

TaskDialog& TaskDialog::icon(QMessageBox::Icon i)
{
  m_icon = i;
  return *this;
}

TaskDialog& TaskDialog::button(TaskDialogButton b)
{
  m_buttons.emplace_back(std::move(b));
  return *this;
}

QMessageBox::StandardButton TaskDialog::exec()
{
  setDialog();
  setWidgets();
  setButtons();
  setDetails();

  m_dialog->adjustSize();

  if (m_dialog->exec() != QDialog::Accepted) {
    return QMessageBox::Cancel;
  }

  return m_result;
}

void TaskDialog::setButtons()
{
  if (m_buttons.empty()) {
    setStandardButtons();
  } else {
    setCommandButtons();
  }

  m_expander.reset(new ExpanderWidget(ui->detailsExpander, ui->detailsWidget));
}

void TaskDialog::setStandardButtons()
{
  ui->standardButtonsPanel->show();
  ui->commandButtonsPanel->hide();
  ui->standardButtons->addButton(QDialogButtonBox::Ok);

  QObject::connect(ui->standardButtons, &QDialogButtonBox::clicked, [&](auto* b){
    m_result = static_cast<QMessageBox::StandardButton>(
      ui->standardButtons->standardButton(b));

    m_dialog->accept();
  });
}

void TaskDialog::setCommandButtons()
{
  ui->standardButtonsPanel->hide();
  ui->commandButtonsPanel->show();

  for (auto&& b : m_buttons) {
    auto* cb = new QCommandLinkButton(b.text, b.description);

    QObject::connect(cb, &QAbstractButton::clicked, [&]{
      m_result = b.button;
      m_dialog->accept();
    });

    ui->commandButtons->layout()->addWidget(cb);
  }
}

void TaskDialog::setDetails()
{
  const QColor bg = detailsColor();

  ui->detailsPanel->setStyleSheet(QString("background-color: rgb(%1, %2, %3)")
    .arg(bg.redF() * 255)
    .arg(bg.greenF() * 255)
    .arg(bg.blueF() * 255));
}

void TaskDialog::setDialog()
{
  m_dialog->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  m_dialog->layout()->setSizeConstraint(QLayout::SetFixedSize);

  m_dialog->setWindowTitle(m_title);
}

void TaskDialog::setWidgets()
{
  ui->main->setText(m_main);
  ui->main->setFont(mainFont());

  ui->content->setText(m_content);
  ui->details->setText(m_details);

  auto icon = standardIcon(QMessageBox::Critical);

  if (icon.isNull()) {
    ui->iconPanel->hide();
  } else {
    ui->iconPanel->show();
    ui->icon->setPixmap(std::move(icon));
  }
}

QFont TaskDialog::mainFont() const
{
  auto f = ui->main->font();

  if (f.pointSizeF() > 0) {
    f.setPointSizeF(f.pointSizeF() * 1.5);
  } else if (f.pixelSize() > 0) {
    f.setPixelSize(static_cast<int>(std::round(f.pixelSize() * 1.5)));
  }

  return f;
}

QColor TaskDialog::detailsColor() const
{
  auto b = std::make_unique<QPushButton>();
  m_dialog->style()->polish(b.get());
  return b->palette().color(b->backgroundRole());
}

QPixmap TaskDialog::standardIcon(QMessageBox::Icon icon) const
{
  QStyle* s = m_dialog->style();
  QIcon i;

  switch (icon) {
    case QMessageBox::Information:
      i = s->standardIcon(QStyle::SP_MessageBoxInformation, 0, m_dialog.get());
      break;
    case QMessageBox::Warning:
      i = s->standardIcon(QStyle::SP_MessageBoxWarning, 0, m_dialog.get());
      break;
    case QMessageBox::Critical:
      i = s->standardIcon(QStyle::SP_MessageBoxCritical, 0, m_dialog.get());
      break;
    case QMessageBox::Question:
      i = s->standardIcon(QStyle::SP_MessageBoxQuestion, 0, m_dialog.get());

    case QMessageBox::NoIcon:  // fall-through
    default:
      break;
  }

  if (i.isNull()) {
    return {};
  }

  QWindow *window = m_dialog->windowHandle();
  if (!window) {
    if (const QWidget *nativeParent = m_dialog->nativeParentWidget())
      window = nativeParent->windowHandle();
  }

  const int iconSize = s->pixelMetric(
    QStyle::PM_MessageBoxIconSize, 0, m_dialog.get());

  return i.pixmap(window, QSize(iconSize, iconSize));
}

} // namespace MOBase
