/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

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

#include <core/ConfigurableWidget.h>

namespace Ui {
class CanStatusWindow;
}

class Backend;
class QTimer;

class CanStatusWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    enum {
        column_driver,
        column_interface,
        column_state,
        column_rx_frames,
        column_rx_errors,
        column_rx_overrun,
        column_tx_frames,
        column_tx_errors,
        column_tx_dropped,
        column_num_warning,
        column_num_passive,
        column_num_busoff,
        column_num_restarts,
        column_count
    };

public:
    explicit CanStatusWindow(QWidget *parent, Backend &backend);
    ~CanStatusWindow();

private slots:
    void beginMeasurement();
    void endMeasurement();
    void update();

private:
    Ui::CanStatusWindow *ui;
    Backend &_backend;

    Backend &backend();
    QTimer *_timer;
};
