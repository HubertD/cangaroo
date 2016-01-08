#include "MeasurementNetwork.h"
#include "MeasurementInterface.h"

MeasurementNetwork::MeasurementNetwork()
{
}

void MeasurementNetwork::cloneFrom(MeasurementNetwork &origin)
{
    _name = origin._name;
    foreach (MeasurementInterface *omi, origin._interfaces) {
        MeasurementInterface *mi = new MeasurementInterface();
        mi->cloneFrom(omi);
        _interfaces.append(mi);
    }
    _canDbs = origin._canDbs;
}

void MeasurementNetwork::addInterface(MeasurementInterface *intf)
{
    _interfaces.append(intf);
}

void MeasurementNetwork::removeInterface(MeasurementInterface *intf)
{
    _interfaces.removeAll(intf);
}

QList<MeasurementInterface *> MeasurementNetwork::interfaces()
{
    return _interfaces;
}

MeasurementInterface *MeasurementNetwork::addCanInterface(CanInterfaceId canif)
{
    MeasurementInterface *mi = new MeasurementInterface();
    mi->setCanInterface(canif);
    addInterface(mi);
    return mi;
}

CanInterfaceIdList MeasurementNetwork::getReferencedCanInterfaces()
{
    CanInterfaceIdList list;
    foreach (MeasurementInterface *mi, _interfaces) {
        list << mi->canInterface();
    }
    return list;
}

void MeasurementNetwork::addCanDb(QSharedPointer<CanDb> candb)
{
    _canDbs.append(candb);
}

QString MeasurementNetwork::name() const
{
    return _name;
}

void MeasurementNetwork::setName(const QString &name)
{
    _name = name;
}

