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

QPixmap standardIcon(QMessageBox::Icon icon, QDialog* mb)
{
  QStyle *style = mb ? mb->style() : QApplication::style();
  int iconSize = style->pixelMetric(QStyle::PM_MessageBoxIconSize, 0, mb);
  QIcon tmpIcon;
  switch (icon) {
    case QMessageBox::Information:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxInformation, 0, mb);
      break;
    case QMessageBox::Warning:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxWarning, 0, mb);
      break;
    case QMessageBox::Critical:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxCritical, 0, mb);
      break;
    case QMessageBox::Question:
      tmpIcon = style->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mb);
    default:
      break;
  }
  if (!tmpIcon.isNull()) {
    QWindow *window = nullptr;
    if (mb) {
      window = mb->windowHandle();
      if (!window) {
        if (const QWidget *nativeParent = mb->nativeParentWidget())
          window = nativeParent->windowHandle();
      }
    }
    return tmpIcon.pixmap(window, QSize(iconSize, iconSize));
  }
  return QPixmap();
}

QMessageBox::StandardButton taskDialog(
  QWidget* parent, const QString& title,
  const QString& mainText, const QString& content, const QString& details,
  std::vector<TaskDialogButton> buttons)
{
  auto* d = new QDialog(parent);
  auto* ui = new Ui::TaskDialog;
  ui->setupUi(d);

  d->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
  d->setWindowTitle(title);
  d->layout()->setSizeConstraint(QLayout::SetFixedSize);

  ui->main->setText(mainText);
  ui->content->setText(content);
  ui->details->setText(details);
  ui->icon->setPixmap(standardIcon(QMessageBox::Critical, d));

  auto f = ui->main->font();
  if (f.pointSizeF() > 0) {
    f.setPointSizeF(f.pointSizeF() * 1.5);
  } else if (f.pixelSize() > 0) {
    f.setPixelSize(f.pixelSize() * 1.5);
  }

  ui->main->setFont(f);

  auto answer = QMessageBox::Cancel;

  if (buttons.empty()) {
    ui->standardButtonsPanel->show();
    ui->commandButtonsPanel->hide();
    ui->standardButtons->addButton(QDialogButtonBox::Ok);
  } else {
    ui->standardButtonsPanel->hide();
    ui->commandButtonsPanel->show();

    for (auto&& b : buttons) {
      auto* cb = new QCommandLinkButton(b.text, b.description);

      QObject::connect(cb, &QAbstractButton::clicked, [&]{
        answer = b.button;
        d->accept();
        });

      ui->commandButtons->layout()->addWidget(cb);
    }
  }

  ExpanderWidget expander(ui->detailsExpander, ui->detailsWidget);

  QColor bg;

  {
    auto cb = std::make_unique<QPushButton>();
    d->style()->polish(cb.get());
    bg = cb->palette().color(cb->backgroundRole());
  }

  ui->detailsPanel->setStyleSheet(QString("background-color: rgb(%1, %2, %3)")
    .arg(bg.redF() * 255)
    .arg(bg.greenF() * 255)
    .arg(bg.blueF() * 255));


  //d->resize(500, d->height());
  //d->setFixedWidth(550);
  d->adjustSize();
  if (d->exec() == QMessageBox::Rejected) {
    return QMessageBox::Cancel;
  }

  return answer;
}

} // namespace MOBase
