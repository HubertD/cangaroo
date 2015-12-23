#include "CanListener.h"

#include "model/CanMessage.h"
#include "CanInterface.h"

CanListener::CanListener(QObject *parent, CanInterface *intf)
  : QObject(parent),
    _intf(intf)
{

}

void CanListener::run()
{
    while (true) {
        CanMessage msg = _intf->readMessage();
        emit messageReceived(_intf->getId(), msg);
    }

}
