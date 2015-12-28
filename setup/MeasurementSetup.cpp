#include "MeasurementSetup.h"

#include <QThread>
#include <drivers/CanListener.h>

MeasurementSetup::MeasurementSetup()
  : QObject()
{

}

MeasurementSetup::~MeasurementSetup()
{
    MeasurementNetwork *network;
    foreach (network, _networks) {
        delete network;
    }
}

MeasurementNetwork *MeasurementSetup::createNetwork()
{
    MeasurementNetwork *network = new MeasurementNetwork();
    _networks.append(network);
    return network;
}

void MeasurementSetup::startMeasurement()
{
    QThread* thread;
    qRegisterMetaType<CanMessage>("CanMessage");

    int i=0;
    foreach (MeasurementNetwork *network, _networks) {
        i++;
        foreach (CanInterface *intf, network->_canInterfaces) {
            intf->setId(i);
            intf->open();

            thread = new QThread;
            CanListener *listener = new CanListener(0, intf);
            listener->moveToThread(thread);
            connect(thread, SIGNAL(started()), listener, SLOT(run()));
            connect(listener, SIGNAL(messageReceived(CanMessage)), &_trace, SLOT(enqueueMessage(CanMessage)));
            thread->start();
        }
    }
}

void MeasurementSetup::stopMeasurement()
{

}

CanTrace *MeasurementSetup::getTrace()
{
    return &_trace;
}

CanDbMessage *MeasurementSetup::findDbMessage(const CanMessage *msg)
{
    CanDbMessage *result = 0;
    if (!msg) { return 0; }

    foreach (MeasurementNetwork *network, _networks) {
        foreach (CanDb *db, network->_canDbs) {
            result = db->getMessageById(msg->getRawId());
            if (result != 0) {
                return result;
            }
        }
    }
    return result;
}

