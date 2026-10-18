/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <core/ConfigurableWidget.h>
#include <core/CanMessage.h>
#include "TraceViewTypes.h"
#include "TraceFilterModel.h"

namespace Ui {
class TraceWindow;
}
class Backend;
class QDomDocument;
class QDomElement;
class QSortFilterProxyModel;
class LinearTraceViewModel;
class AggregatedTraceViewModel;
class UnifiedTraceViewModel;


class TraceWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    typedef enum mode {
        mode_aggregated,
        mode_unified
    } mode_t;

    explicit TraceWindow(QWidget *parent, Backend &backend);
    ~TraceWindow();

    void setMode(mode_t mode);
    void setTimestampMode(int mode);

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);

public slots:
    void addMessage(const CanMessage &msg);
private slots:
    void onRowsInserted(const QModelIndex & parent, int first, int last);
    void onTraceTextColorChanged(QColor color);

    void on_cbTimestampMode_currentIndexChanged(int index);
    void on_cbFilterChanged(void);

    void on_cbTraceClearpushButton(void);
    void on_cbViewMode_currentIndexChanged(int index);

private:
    Ui::TraceWindow *ui;
    Backend *_backend;
    mode_t _mode;
    timestamp_mode_t _timestampMode;

    enum Category {
        Cat_Aggregated = 0,
        Cat_UDS = 1,
        Cat_J1939 = 2,
        Cat_Count = 3
    };

    TraceFilterModel * _filterModels[Cat_Count];
    UnifiedTraceViewModel *_viewModels[Cat_Count];
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
    QSortFilterProxyModel *_aggregatedProxyModel;
    TraceFilterModel * _aggMonitorFilterModel; // Existing aggregated monitor mode
};
