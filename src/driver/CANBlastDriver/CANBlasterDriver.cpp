/*

  Copyright (c) 2022 Ethan Zonca

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


#include "CANBlasterDriver.h"
#include "CANBlasterInterface.h"
#include <core/Backend.h>
#include <driver/GenericCanSetupPage.h>

#include <errno.h>
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <QCoreApplication>
#include <QDebug>
#include <QtNetwork/QUdpSocket>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>

CANBlasterDriver::CANBlasterDriver(Backend &backend)
  : CanDriver(backend),
    setupPage(new GenericCanSetupPage())
{
    QObject::connect(&backend, SIGNAL(onSetupDialogCreated(SetupDialog&)), setupPage, SLOT(onSetupDialogCreated(SetupDialog&)));
}

CANBlasterDriver::~CANBlasterDriver() {
}

bool CANBlasterDriver::update() {

    deleteAllInterfaces();

    // TODO: Listen for multicast packets for discovery of canblaster servers
    QUdpSocket udpSocket;
    QHostAddress groupAddress;
    groupAddress.setAddress("239.255.43.21");
    udpSocket.bind(QHostAddress::AnyIPv4, 20000, QUdpSocket::ShareAddress);
    udpSocket.joinMulticastGroup(groupAddress);

    // Record start time
    qint64 start_time_ms = QDateTime::currentMSecsSinceEpoch();

    fprintf(stderr, "CANblaster: start listen\r\n");

    QList<QString> detected_servers;

    while(1)
    {
        while (udpSocket.hasPendingDatagrams())
        {
            QNetworkDatagram res = udpSocket.receiveDatagram(1024);
            if(res.isValid())
            {
                QByteArray asd = res.data();
                qDebug() << asd;
                QJsonDocument document = QJsonDocument::fromJson(res.data());
                QJsonObject rootObj = document.object();


                if(rootObj.length() == 2 &&
                   rootObj["protocol"].toString() == "CANblaster" &&
                   rootObj["version"].toInt() == 1)
                {
                    if(!detected_servers.contains(res.senderAddress().toString()))
                        detected_servers.append(res.senderAddress().toString());
                }
                else
                {
                    fprintf(stderr, "Invalid CANblaster server. Protocol: %s  Version: %d \r\n", rootObj["protocol"].toString().toStdString().c_str(), rootObj["version"].toInt());
                }
            }
        }

        qint64 current_time_ms = QDateTime::currentMSecsSinceEpoch();

        // Iterate until timer expires (2 seconds)
        if ((current_time_ms - start_time_ms) > 2000)
            break;
    }
    fprintf(stderr, "CANblaster: stop listen\r\n");

    fprintf(stderr, "Found %lld servers: \r\n", detected_servers.length());
    int interface_cnt = 0;

    for(QString server: detected_servers)
    {
        fprintf(stderr, "  - %s\r\n", server.toStdString().c_str());
        createOrUpdateInterface(interface_cnt++, server, false);
    }


    return true;
}

QString CANBlasterDriver::getName() {
    return "CANblaster";
}



CANBlasterInterface *CANBlasterDriver::createOrUpdateInterface(int index, QString name, bool fd_support) {

    foreach (CanInterface *intf, getInterfaces()) {
        CANBlasterInterface *scif = dynamic_cast<CANBlasterInterface*>(intf);
		if (scif->getIfIndex() == index) {
			scif->setName(name);
            return scif;
		}
	}


    CANBlasterInterface *scif = new CANBlasterInterface(this, index, name, fd_support);
    addInterface(scif);
    return scif;
}
