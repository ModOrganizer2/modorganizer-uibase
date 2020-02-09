#include "widgetutility.h"
#include "eventfilter.h"
#include "log.h"
#include <QHeaderView>
#include <QCheckBox>
#include <QWidgetAction>

namespace MOBase
{

void onHeaderContextMenu(QTreeView* view, const QPoint& pos)
{
  auto* header = view->header();

  QMenu menu;

  // display a list of all headers as checkboxes
  QAbstractItemModel* model = header->model();

  for (int i = 1; i < model->columnCount(); ++i) {
    const QString columnName =
      model->headerData(i, Qt::Horizontal).toString();

    auto* checkBox = new QCheckBox(&menu);
    checkBox->setText(columnName);
    checkBox->setChecked(!header->isSectionHidden(i));

    auto* checkableAction = new QWidgetAction(&menu);
    checkableAction->setDefaultWidget(checkBox);

    QObject::connect(checkBox, &QCheckBox::clicked, [=]{
      header->setSectionHidden(i, !checkBox->isChecked());
    });

    menu.addAction(checkableAction);
  }

  menu.exec(header->viewport()->mapToGlobal(pos));
}

void setCustomizableColumns(QTreeView* view)
{
  auto* header = view->header();

  header->setSectionsMovable(true);
  header->setContextMenuPolicy(Qt::CustomContextMenu);

  QObject::connect(
    header, &QWidget::customContextMenuRequested,
    view, [view](auto&& pos){ onHeaderContextMenu(view, pos); });
}

} // namespace
