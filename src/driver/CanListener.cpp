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

#include <core/Backend.h>
#include <core/CanTrace.h>
#include <core/CanMessage.h>
#include "CanInterface.h"

#include <QMessageBox>

CanListener::CanListener(QObject *parent, Backend &backend, CanInterface &intf)
  : QObject(parent),
    _backend(backend),
    _intf(intf),
    _shouldBeRunning(true),
    _openComplete(false)
{
    _thread = new QThread();
}

CanListener::~CanListener()
{
    delete _thread;
}

CanInterfaceId CanListener::getInterfaceId()
{
    return _intf.getId();
}

CanInterface &CanListener::getInterface()
{
    return _intf;
}

void CanListener::run()
{
    // Note: open and close done from run() so all operations take place in the same thread
    //CanMessage msg;
    QList<CanMessage> rxMessages;
    CanTrace *trace = _backend.getTrace();

    _intf.open();

    qRegisterMetaType<log_level_t >("log_level_t");
    log_info(QString(tr("interface: %1, Version: %2")).arg(_intf.getName(),_intf.getVersion()));

    _openComplete = true;
    while (_shouldBeRunning) {
        if (_intf.readMessage(rxMessages, 500)) {
            for(const CanMessage &msg: std::as_const(rxMessages))
            {
                trace->enqueueMessage(msg, false);
            }
            rxMessages.clear();
        }
        else if(_intf.isOpen() == false)
        {
            log_error(QString(tr("Error on interface: %1, Closed!!!")).arg(_intf.getName()));
            rxMessages.clear();
            break;

        }
        else
        {
            rxMessages.clear();
        }
    }
    _intf.close();
    _thread->quit();
}

void CanListener::startThread()
{
    moveToThread(_thread);
    connect(_thread, SIGNAL(started()), this, SLOT(run()));
    _thread->start();

    // Wait for interface to be open before returning so that beginMeasurement is emitted after interface open
    while(!_openComplete)
      QThread().usleep(250);
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
