#include "MeasurementSetup.h"

#include <QThread>
#include <QMetaType>

#include <model/CanTrace.h>
#include <model/CanMessage.h>
#include <model/MeasurementNetwork.h>

MeasurementSetup::MeasurementSetup(QObject *parent)
  : QObject(parent)
{
}

MeasurementSetup::~MeasurementSetup()
{
    qDeleteAll(_networks);
}

void MeasurementSetup::cloneFrom(MeasurementSetup &origin)
{
    qDeleteAll(_networks);
    _networks.clear();

    foreach (MeasurementNetwork *network, origin._networks) {
        MeasurementNetwork *network_copy = new MeasurementNetwork();
        network_copy->cloneFrom(*network);
        _networks.append(network_copy);
    }
}


MeasurementNetwork *MeasurementSetup::createNetwork()
{
    MeasurementNetwork *network = new MeasurementNetwork();
    _networks.append(network);
    return network;
}


CanDbMessage *MeasurementSetup::findDbMessage(const CanMessage &msg)
{
    CanDbMessage *result = 0;

    foreach (MeasurementNetwork *network, _networks) {
        foreach (pCanDb db, network->_canDbs) {
            result = db->getMessageById(msg.getRawId());
            if (result != 0) {
                return result;
            }
        }
    }
    return result;
}

QString MeasurementSetup::getInterfaceName(const CanInterface &interface)
{
    return interface.getName();
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

