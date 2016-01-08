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
