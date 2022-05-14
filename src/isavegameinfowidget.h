#ifndef ISAVEGAMEINFOWIDGET_H
#define ISAVEGAMEINFOWIDGET_H

#include <QFile>
#include <QWidget>

#include "isavegame.h"

namespace MOBase
{

/**
 * @brief Base class for a save game info widget.
 *
 * This supports something or other
 */
class ISaveGameInfoWidget : public QWidget
{
public:
  ISaveGameInfoWidget(QWidget* parent = 0) : QWidget(parent) {}

  /**
   * @brief Set the save file to display in the widget.
   */
  virtual void setSave(ISaveGame const&) = 0;

  virtual ~ISaveGameInfoWidget() {}
};

}  // namespace MOBase

#endif  // ISAVEGAMEINFOWIDGET_H
