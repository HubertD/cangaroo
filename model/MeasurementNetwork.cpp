#include "MeasurementNetwork.h"
#include "MeasurementInterface.h"
#include <Backend.h>

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

bool MeasurementNetwork::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    root.setAttribute("name", _name);

    QDomElement interfacesNode = xml.createElement("interfaces");
    foreach (MeasurementInterface *intf, _interfaces) {
        QDomElement intfNode = xml.createElement("interface");
        if (!intf->saveXML(backend, xml, intfNode)) {
            return false;
        }
        interfacesNode.appendChild(intfNode);
    }
    root.appendChild(interfacesNode);

    QDomElement candbsNode = xml.createElement("databases");
    foreach (pCanDb candb, _canDbs) {
        QDomElement dbNode = xml.createElement("database");
        if (!candb->saveXML(backend, xml, dbNode)) {
            return false;
        }
        candbsNode.appendChild(dbNode);
    }
    root.appendChild(candbsNode);


    return true;
}

