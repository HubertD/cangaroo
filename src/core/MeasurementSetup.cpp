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

#include "MeasurementSetup.h"

#include <QThread>
#include <QMetaType>

#include <core/CanTrace.h>
#include <core/CanMessage.h>
#include <core/MeasurementNetwork.h>

MeasurementSetup::MeasurementSetup(QObject *parent)
  : QObject(parent)
{
}

MeasurementSetup::~MeasurementSetup()
{
    qDeleteAll(_networks);
}

void MeasurementSetup::clear()
{
    qDeleteAll(_networks);
    _networks.clear();
    emit onSetupChanged();
}

void MeasurementSetup::cloneFrom(MeasurementSetup &origin)
{
    clear();
    foreach (MeasurementNetwork *network, origin._networks) {
        MeasurementNetwork *network_copy = new MeasurementNetwork();
        network_copy->cloneFrom(*network);
        _networks.append(network_copy);
    }
    emit onSetupChanged();
}

bool MeasurementSetup::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    foreach (MeasurementNetwork *network, _networks) {
        QDomElement networkNode = xml.createElement("network");
        if (!network->saveXML(backend, xml, networkNode)) {
            return false;
        }
        root.appendChild(networkNode);
    }
    return true;
}

bool MeasurementSetup::loadXML(Backend &backend, QDomElement &el)
{
    clear();

    QDomNodeList networks = el.elementsByTagName("network");
    for (int i=0; i<networks.length(); i++) {
        MeasurementNetwork *network = createNetwork();
        if (!network->loadXML(backend, networks.item(i).toElement())) {
            return false;
        }
    }

    emit onSetupChanged();
    return true;
}


MeasurementNetwork *MeasurementSetup::createNetwork()
{
    MeasurementNetwork *network = new MeasurementNetwork();
    _networks.append(network);
    return network;
}

void MeasurementSetup::removeNetwork(MeasurementNetwork *network)
{
    if (_networks.removeAll(network) > 0) {
        delete network;
    }
}


CanDbMessage *MeasurementSetup::findDbMessage(const CanMessage &msg) const
{
    CanInterfaceId msgIfId = msg.getInterfaceId();

    foreach (MeasurementNetwork *network, _networks) {
        // Enforce network context: Only search DBs in the network that contains this interface
        if (network->getReferencedCanInterfaces().contains(msgIfId)) {
            foreach (pCanDb db, network->_canDbs) {
                CanDbMessage *result = db->getMessageById(msg.getRawId());
                if (result != 0) {
                    return result;
                }
                // J1939: frames carry a varying source address (and priority) in the
                // CAN id, so an exact raw-id match fails. Fall back to matching by PGN.
                if (db->isJ1939() && msg.isExtended()) {
                    uint32_t pgn = CanMessage::j1939Pgn(msg.getId());
                    foreach (CanDbMessage *dbMsg, db->getMessageList()) {
                        if (CanMessage::j1939Pgn(dbMsg->getRaw_id() & 0x1FFFFFFF) == pgn) {
                            return dbMsg;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

QString MeasurementSetup::getInterfaceName(const CanInterface &interface) const
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

MeasurementNetwork *MeasurementSetup::getNetworkByName(QString name) const
{
    foreach (MeasurementNetwork *network, _networks) {
        if (network->name() == name) {
            return network;
        }
    }
    return 0;
}

QList<MeasurementNetwork *> MeasurementSetup::getNetworks()
{
    return _networks;
}

