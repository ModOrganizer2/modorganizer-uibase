#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QObject>
#include <QLineEdit>
#include <QToolButton>
#include <QList>
#include <QAbstractItemView>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
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

protected:
  bool filterAcceptsRow(int row, const QModelIndex& parent) const override;
  void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
  FilterWidget& m_filter;

  bool columnMatches(
    int sourceRow, const QModelIndex& sourceParent,
    int c, const QRegularExpression& what) const;
};


class QDLLEXPORT FilterWidget : public QObject
{
  Q_OBJECT;

public:
  struct Options
  {
    bool useRegex = false;
    bool regexCaseSensitive = false;
    bool regexExtended = false;
  };

  using predFun = std::function<bool (const QRegularExpression& what)>;

  FilterWidget();

  static void setOptions(const Options& o);
  static Options options();

  void setEdit(QLineEdit* edit);
  void setList(QAbstractItemView* list);
  void clear();
  bool empty() const;

  void setUseSourceSort(bool b);
  bool useSourceSort() const;

  void setFilterColumn(int i);
  int filterColumn() const;

  FilterWidgetProxyModel* proxyModel();

  QModelIndex map(const QModelIndex& index);

  bool matches(predFun pred) const;

signals:
  void aboutToChange(const QString& oldFilter, const QString& newFilter);
  void changed(const QString& oldFilter, const QString& newFilter);

private:
  using Compiled = QList<QList<QRegularExpression>>;

  QLineEdit* m_edit;
  QAbstractItemView* m_list;
  FilterWidgetProxyModel* m_proxy;
  EventFilter* m_eventFilter;
  QToolButton* m_clear;
  QString m_text;
  Compiled m_compiled;
  bool m_valid;
  bool m_useSourceSort;
  int m_filterColumn;

  void unhook();
  void createClear();
  void hookEvents();
  void repositionClearButton();

  void onTextChanged();
  void onResized();
  void onContextMenu(QObject*, QContextMenuEvent* e);

  void set(const QString& text);
  void update();
  void compile();
};

} // namespace

#endif // FILTERWIDGET_H
