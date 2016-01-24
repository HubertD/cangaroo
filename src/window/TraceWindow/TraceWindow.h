/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

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

#include <MdiWindow.h>
#include "TraceViewTypes.h"

namespace Ui {
class TraceWindow;
}

class QDomDocument;
class QDomElement;
class QSortFilterProxyModel;
class LinearTraceViewModel;
class AggregatedTraceViewModel;
class Backend;

class TraceWindow : public MdiWindow
{
    Q_OBJECT

public:
    typedef enum mode {
        mode_linear,
        mode_aggregated
    } mode_t;

    explicit TraceWindow(QWidget *parent, Backend &backend);
    ~TraceWindow();

    void setMode(mode_t mode);
    void setAutoScroll(bool doAutoScroll);
    void setTimestampMode(int mode);

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);

public slots:
    void rowsInserted(const QModelIndex & parent, int first, int last);

private slots:
    void on_cbAggregated_stateChanged(int i);
    void on_cbAutoScroll_stateChanged(int i);

    void on_cbTimestampMode_currentIndexChanged(int index);

private:
    Ui::TraceWindow *ui;
    Backend *_backend;
    mode_t _mode;
    bool _doAutoScroll;
    timestamp_mode_t _timestampMode;

    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
    QSortFilterProxyModel *_aggregatedProxyModel;
    QSortFilterProxyModel *_linearProxyModel;
};
