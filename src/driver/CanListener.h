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

#include <QThread>
#include <QObject>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

//class QThread;
class CanMessage;
class Backend;

class CanListener : public QObject
{
    Q_OBJECT

public:
    explicit CanListener(QObject *parent, Backend &backend, CanInterface &intf);
    virtual ~CanListener();

    CanInterfaceId getInterfaceId();
    CanInterface &getInterface();

signals:
    void messageReceived(const CanMessage &msg);

public slots:
    void run();

    void startThread();
    void requestStop();
    void waitFinish();

private:
    Backend &_backend;
    CanInterface &_intf;
    bool _shouldBeRunning;
    bool _openComplete;
    QThread *_thread;
};
