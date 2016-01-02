#include "MeasurementNetwork.h"

MeasurementNetwork::MeasurementNetwork()
{
}

void MeasurementNetwork::addCanInterface(CanInterface *intf)
{
    _canInterfaces.append(intf);
}

void MeasurementNetwork::addCanDb(CanDb *candb)
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

