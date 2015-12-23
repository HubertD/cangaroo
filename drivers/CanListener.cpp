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
    CanMessage msg;
    while (true) {
        if (_intf->readMessage(msg)) {
            emit messageReceived(msg);
        }
    }

}
