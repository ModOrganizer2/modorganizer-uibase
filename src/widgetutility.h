#ifndef UIBASE_WIDGETUTILITY_INCLUDED
#define UIBASE_WIDGETUTILITY_INCLUDED

#include "dllimport.h"
#include <QAbstractItemView>

namespace MOBase
{

  constexpr auto EnabledColumnRole = Qt::UserRole + 1;

  QDLLEXPORT void setCustomizableColumns(QTreeView* view);

} // namespace

#endif // UIBASE_WIDGETUTILITY_INCLUDED
