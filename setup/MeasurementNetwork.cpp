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

