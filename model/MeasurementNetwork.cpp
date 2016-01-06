#include "MeasurementNetwork.h"

MeasurementNetwork::MeasurementNetwork()
{
}

void MeasurementNetwork::cloneFrom(MeasurementNetwork &origin)
{
    _name = origin._name;
    _canInterfaces = origin._canInterfaces;
    _canDbs = origin._canDbs;
}

void MeasurementNetwork::removeCanInterface(pCanInterface intf)
{
    _canInterfaces.removeAll(intf);
}

void MeasurementNetwork::removeCanInterface(CanInterface *intf)
{
    foreach (pCanInterface pci, _canInterfaces) {
        if (pci.data() == intf) {
            removeCanInterface(pci);
            break;
        }
    }
}

void MeasurementNetwork::addCanInterface(QSharedPointer<CanInterface> intf)
{
    _canInterfaces.append(intf);
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

