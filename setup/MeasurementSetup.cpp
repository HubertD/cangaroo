#include "MeasurementSetup.h"

#include <QThread>
#include <drivers/CanListener.h>
#include <QMetaType>

#include <model/CanTrace.h>
#include <setup/MeasurementNetwork.h>
#include <model/CanMessage.h>

MeasurementSetup::MeasurementSetup(QObject *parent)
  : QObject(parent)
{
    _trace = new CanTrace();
}

MeasurementSetup::~MeasurementSetup()
{
    foreach (MeasurementNetwork *network, _networks) {
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

    log(log_level_info, "starting measurement");
    int i=0;
    foreach (MeasurementNetwork *network, _networks) {
        i++;
        foreach (CanInterface *intf, network->_canInterfaces) {
            intf->setId(i);
            intf->open();

            log(log_level_info, QString().sprintf("listening on interface %s", intf->getName().toStdString().c_str()));

            thread = new QThread;
            CanListener *listener = new CanListener(0, intf);
            listener->moveToThread(thread);
            connect(thread, SIGNAL(started()), listener, SLOT(run()));
            connect(listener, SIGNAL(messageReceived(CanMessage)), _trace, SLOT(enqueueMessage(CanMessage)));
            thread->start();
        }
    }
}

void MeasurementSetup::stopMeasurement()
{
    log(log_level_info, "stopping measurement");
}

CanTrace *MeasurementSetup::getTrace()
{
    return _trace;
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

QString MeasurementSetup::getInterfaceName(CanInterface *interface)
{
    if (interface) {
        return QString(interface->getName());
    } else {
        return "none";
    }
}

void MeasurementSetup::log(log_level_t level, QString s)
{
    emit appendLog(level, s);
}

