#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QObject>
#include <QLineEdit>
#include <QToolButton>
#include <QList>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include "dllimport.h"

namespace MOBase {

class EventFilter;
class FilterWidget;

class QDLLEXPORT FilterWidgetProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT;

public:
  FilterWidgetProxyModel(FilterWidget& fw, QWidget* parent=nullptr);
  using QSortFilterProxyModel::invalidateFilter;

  void setUseSourceSort(bool b);
  bool useSourceSort() const;

protected:
  bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
  FilterWidget& m_filter;
  bool m_useSourceSort;
};


class QDLLEXPORT FilterWidget : public QObject
{
  Q_OBJECT;

public:
  using predFun = std::function<bool (const QString& what)>;

  FilterWidget();

  void setEdit(QLineEdit* edit);
  void setList(QAbstractItemView* list);
  void clear();
  bool empty() const;

  void setUseSourceSort(bool b);
  bool useSourceSort() const;

  FilterWidgetProxyModel* proxyModel();

  QModelIndex map(const QModelIndex& index);

  bool matches(predFun pred) const;

signals:
  void aboutToChange(const QString& oldFilter, const QString& newFilter);
  void changed(const QString& oldFilter, const QString& newFilter);

private:
  QLineEdit* m_edit;
  QAbstractItemView* m_list;
  FilterWidgetProxyModel* m_proxy;
  EventFilter* m_eventFilter;
  QToolButton* m_clear;
  QString m_text;
  QList<QList<QString>> m_compiled;

  void unhook();
  void createClear();
  void hookEvents();
  void repositionClearButton();

  void onTextChanged();
  void onResized();

  void compile();
};

} // namespace

#endif // FILTERWIDGET_H
