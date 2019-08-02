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


#include "textviewer.h"
#include "ui_textviewer.h"
#include "utility.h"
#include "report.h"
#include "finddialog.h"
#include "log.h"
#include <QFile>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QShortcutEvent>
#include <QAction>

namespace MOBase {


TextViewer::TextViewer(const QString &title, QWidget *parent)
  : QDialog(parent), ui(new Ui::TextViewer), m_FindDialog(nullptr)
{
  ui->setupUi(this);
  setWindowTitle(title);
  m_EditorTabs = findChild<QTabWidget*>("editorTabs");

}


TextViewer::~TextViewer()
{
  delete ui;
}

void TextViewer::closeEvent(QCloseEvent *event)
{
  if (!m_Modified.empty()) {
    for (std::set<QTextEdit*>::iterator iter = m_Modified.begin(); iter != m_Modified.end(); ++iter) {
      QMessageBox::StandardButton res = QMessageBox::question(this, tr("Save changes?"),
              tr("Do you want to save changes to %1?").arg((*iter)->documentTitle()),
              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      if (res == QMessageBox::Yes) {
        saveFile(*iter);
      } else if (res == QMessageBox::Cancel) {
        event->ignore();
        break;
      }
    }
  }
}


void TextViewer::find()
{
   if (!m_FindDialog) {
     m_FindDialog = new FindDialog(this);
     connect(m_FindDialog, SIGNAL(findNext()), this, SLOT(findNext()));
     connect(m_FindDialog, SIGNAL(patternChanged(QString)), this, SLOT(patternChanged(QString)));
   }

   m_FindDialog->show();
   m_FindDialog->raise();
   m_FindDialog->activateWindow();
}


void TextViewer::patternChanged(QString newPattern)
{
  m_FindPattern = newPattern;
}


void TextViewer::findNext()
{
  if (m_FindPattern.length() == 0) {
    return;
  }

  QWidget* currentPage = m_EditorTabs->currentWidget();
  QTextEdit *editor = currentPage->findChild<QTextEdit*>("editorView");

  editor->find(m_FindPattern);
}


bool TextViewer::eventFilter(QObject* object, QEvent *event)
{
  if (event->type() == QEvent::ShortcutOverride) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->matches(QKeySequence::Find)) {
      find();
    } else if (keyEvent->matches(QKeySequence::FindNext)) {
      findNext();
    }
  }
  return QDialog::eventFilter(object, event);
}


void TextViewer::setDescription(const QString &description)
{
  QLabel *descriptionLabel = findChild<QLabel*>("descriptionLabel");
  descriptionLabel->setText(description);
}


void TextViewer::saveFile(const QTextEdit *editor)
{
  bool write = true;
  QFile file(editor->documentTitle());
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    write = false;
    if (QMessageBox::question(QApplication::activeModalWidget(),QApplication::tr("File is read-only"),
          QApplication::tr("Mod Organizer is attempting to write to \"%1\" which is currently set to read-only. "
          "Clear the read-only flag to allow the write?").arg(file.fileName())) == QMessageBox::Yes) {
      log::warn("{} is read-only.  Attempting to clear read-only flag.", file.fileName());
      file.setPermissions(file.permissions() | QFile::WriteUser);
    } else {
      log::warn("{} is read-only.  User denied clearing the read-only flag.", file.fileName());
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      reportError(tr("failed to write to %1").arg(editor->documentTitle()));
    } else {
      write = true;
    }
  }

  if (write) {
    file.write(editor->toPlainText().toUtf8().replace('\n', "\r\n"));
    file.close();
  }
}


void TextViewer::saveFile()
{
  QWidget* currentPage = m_EditorTabs->currentWidget();
  QTextEdit *editor = currentPage->findChild<QTextEdit*>("editorView");
  saveFile(editor);

  m_Modified.erase(editor);
}


void TextViewer::modified()
{
  QWidget* currentPage = m_EditorTabs->currentWidget();
  QTextEdit *editor = currentPage->findChild<QTextEdit*>("editorView");

  m_Modified.insert(editor);
}


void TextViewer::addFile(const QString &fileName, bool writable)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly)) {
    throw MyException(tr("file not found: %1").arg(fileName));
  }
  QByteArray temp = file.readAll();

  QWidget *page = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(page);
  QTextEdit *editor = new QTextEdit(page);
  editor->setAcceptRichText(false);
  editor->setPlainText(QString(temp));
  editor->setLineWrapMode(QTextEdit::NoWrap);
  editor->setObjectName("editorView");
  editor->setDocumentTitle(fileName);
  editor->installEventFilter(this);
  editor->setReadOnly(!writable);

  // add hotkeys for searching through the document
  QAction *findAction = new QAction(QString("&Find"), editor);
  findAction->setShortcut(QKeySequence::Find);
  editor->addAction(findAction);
  QAction *findNextAction = new QAction(QString("Find &Next"), editor);
  findAction->setShortcut(QKeySequence::FindNext);
  editor->addAction(findNextAction);

  layout->addWidget(editor);
  if (writable) {
    QPushButton *saveBtn = new QPushButton(tr("Save"), page);
    layout->addWidget(saveBtn);
    connect(saveBtn, SIGNAL(clicked()), this, SLOT(saveFile()));
    connect(editor, SIGNAL(textChanged()), this, SLOT(modified()));
  }
  page->setLayout(layout);
  m_EditorTabs->addTab(page, QFileInfo(fileName).fileName());
}
} // namespace MOBase
