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


#include "SLCANDriver.h"
#include "SLCANInterface.h"
#include <core/Backend.h>
#include <driver/GenericCanSetupPage.h>

#include <unistd.h>
#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

SLCANDriver::SLCANDriver(Backend &backend)
  : CanDriver(backend),
    setupPage(new GenericCanSetupPage())
{
    QObject::connect(&backend, SIGNAL(onSetupDialogCreated(SetupDialog&)), setupPage, SLOT(onSetupDialogCreated(SetupDialog&)));
}

SLCANDriver::~SLCANDriver()
{
}

bool SLCANDriver::update()
{
    deleteAllInterfaces();

    int interface_cnt = 0;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // fprintf(stderr, "Name : %s \r\n",  info.portName().toStdString().c_str());
        // fprintf(stderr, "   Description : %s \r\n", info.description().toStdString().c_str());
        // fprintf(stderr, "   Manufacturer: %s \r\n", info.manufacturer().toStdString().c_str());

        if(info.vendorIdentifier() == 0xad50 && info.productIdentifier() == 0x60C4)
        {
            std::cout << "   ++ CANable 1.0 or similar ST USB CDC device detected" << std::endl;

            // Create new slcan interface without FD support
            _manufacturer = SLCANInterface::CANable;
            createOrUpdateInterface(interface_cnt, info.portName(), false, _manufacturer);
            interface_cnt++;
        }
        else if(info.vendorIdentifier() == 0x0403 && info.productIdentifier() == 0x6015)
        {
            std::cout << "   ++ CANable 1.0 or similar ST USB CDC device detected" << std::endl;

            // Create new slcan interface without FD support
            _manufacturer = SLCANInterface::CANable;
            createOrUpdateInterface(interface_cnt, info.portName(), false, _manufacturer);
            interface_cnt++;
        }
        else if(info.vendorIdentifier() == 0x16D0 && info.productIdentifier() == 0x117E)
        {
            std::cout << "   ++ CANable 2.0 detected" << std::endl;

            _manufacturer = SLCANInterface::CANable;
            // Create new slcan interface with FD support
            createOrUpdateInterface(interface_cnt, info.portName(), true, _manufacturer);
            interface_cnt++;
        }
        else if(info.vendorIdentifier() == 1155 && info.productIdentifier() == 22336 && info.serialNumber().startsWith("AAA"))
        {
            std::cout << "   ++ WeAct Studio USB2CAN detected" << std::endl;
            std::cout << "   ++ " << info.serialNumber().toStdString().c_str() << std::endl;

            _manufacturer = SLCANInterface::WeActStudio;
            // Create new slcan interface with FD support
            createOrUpdateInterface(interface_cnt, info.portName(), true, _manufacturer);
            interface_cnt++;
        }
        else
        {
            //std::cout << "   !! This is not a SLCAN device!" << std::endl;
        }
    }

    return true;
}

QString SLCANDriver::getName()
{
    return "SLCAN";
}

SLCANInterface *SLCANDriver::createOrUpdateInterface(int index, QString name, bool fd_support, uint32_t manufacturer)
{
    foreach (CanInterface *intf, getInterfaces())
    {
        SLCANInterface *scif = dynamic_cast<SLCANInterface*>(intf);
        if (scif->getIfIndex() == index)
        {
			scif->setName(name);
            return scif;
		}
	}

    SLCANInterface *scif = new SLCANInterface(this, index, name, fd_support, manufacturer);
    addInterface(scif);

    return scif;
}
