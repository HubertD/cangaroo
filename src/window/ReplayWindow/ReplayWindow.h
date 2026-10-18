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
#include <core/Backend.h>
#include <QVector>
#include <core/CanMessage.h>
#include <core/CanReplayer.h>

namespace Ui {
class ReplayWindow;
}

class ReplayWindow : public ConfigurableWidget
{
    Q_OBJECT

public:
    explicit ReplayWindow(QWidget *parent, Backend &backend);
    ~ReplayWindow() override;

    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root) override;
    bool loadXML(Backend &backend, QDomElement &el) override;

private slots:
    void on_btnBrowse_clicked();
    void on_btnPlay_clicked();
    void on_btnPause_clicked();
    void on_btnStop_clicked();
    void on_chkLoop_stateChanged(int arg1);
    void on_cbInterface_currentIndexChanged(int index);

    void onProgressUpdated(int index, int total);
    void onReplayFinished();

public slots:
    void refreshInterfaces();

private:
    Ui::ReplayWindow *ui;
    Backend &_backend;
    CanReplayer *_replayer;
    QVector<CanMessage> _loadedMessages;

    void loadLogFile(const QString& filename);
};
