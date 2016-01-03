#include "MeasurementSetup.h"

#include <QThread>
#include <QMetaType>

#include <model/CanTrace.h>
#include <model/CanMessage.h>
#include <setup/MeasurementNetwork.h>

MeasurementSetup::MeasurementSetup(QObject *parent)
  : QObject(parent)
{
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

int MeasurementSetup::countNetworks() const
{
    return _networks.length();
}

MeasurementNetwork *MeasurementSetup::getNetwork(int index) const
{
    return _networks.value(index);
}

QList<MeasurementNetwork *> MeasurementSetup::getNetworks()
{
    return _networks;
}

