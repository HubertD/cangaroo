/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "MeasurementNetwork.h"
#include "MeasurementInterface.h"

#include <core/Backend.h>


MeasurementNetwork::MeasurementNetwork()
{
}

void MeasurementNetwork::cloneFrom(MeasurementNetwork &origin)
{
    _name = origin._name;
    foreach (MeasurementInterface *omi, origin._interfaces) {
        MeasurementInterface *mi = new MeasurementInterface();
        mi->cloneFrom(*omi);
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

void MeasurementNetwork::reloadCanDbs(Backend *backend)
{
    foreach(pCanDb db, _canDbs)
    {
        db = backend->loadDbc(db->getPath());
    }
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

bool MeasurementNetwork::loadXML(Backend &backend, QDomElement el)
{
    setName(el.attribute("name", "unnamed network"));

    QDomNodeList ifList = el.firstChildElement("interfaces").elementsByTagName("interface");
    for (int i=0; i<ifList.length(); i++) {
        QDomElement elIntf = ifList.item(i).toElement();
        QString driverName = elIntf.attribute("driver");
        QString deviceName = elIntf.attribute("name");
        CanInterface *intf = backend.getInterfaceByDriverAndName(driverName, deviceName);
        if (intf) {
            MeasurementInterface *mi = addCanInterface(intf->getId());
            mi->loadXML(backend, elIntf);
        } else {
            log_error(QString("Could not find interface %1/%2, which is referenced in the workspace config file.").arg(driverName, deviceName));
        }
    }


    QDomNodeList dbList = el.firstChildElement("databases").elementsByTagName("database");
    for (int i=0; i<dbList.length(); i++) {
        QDomElement elDb = dbList.item(i).toElement();
        QString filename = elDb.attribute("filename", QString());
        if (!filename.isEmpty()) {
            addCanDb(backend.loadDbc(filename));
        } else {
            log_error(QString("Unable to load CanDB: %1").arg(filename));
        }
    }

    return true;
}

