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

#include "CanListener.h"

#include <QThread>
#include <QDebug>

#include "model/CanMessage.h"
#include "CanInterface.h"

CanListener::CanListener(QObject *parent, CanInterface *intf)
  : QObject(parent),
    _intf(intf),
    _shouldBeRunning(true)
{
    _thread = new QThread();
}

CanListener::~CanListener()
{
    delete _thread;
}

CanInterfaceId CanListener::getInterfaceId()
{
    return _intf->getId();
}

CanInterface *CanListener::getInterface()
{
    return _intf;
}

void CanListener::run()
{
    CanMessage msg;
    while (_shouldBeRunning) {
        if (_intf->readMessage(msg, 1000)) {
            emit messageReceived(msg);
        }
    }
    _thread->quit();
}

void CanListener::startThread()
{
    moveToThread(_thread);
    connect(_thread, SIGNAL(started()), this, SLOT(run()));
    _thread->start();
}

void CanListener::requestStop()
{
    _shouldBeRunning = false;
}

void CanListener::waitFinish()
{
    requestStop();
    _thread->wait();
}
