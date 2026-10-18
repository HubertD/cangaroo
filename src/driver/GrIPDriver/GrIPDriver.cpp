/*

  Copyright (c) 2024 Schildkroet

  This file is part of CANgaroo.

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

#include "GrIPDriver.h"
#include "GrIPInterface.h"
#include <core/Backend.h>
#include <driver/GenericCanSetupPage.h>

#include <unistd.h>
#include <iostream>

#include <QCoreApplication>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QThread>


GrIPDriver::GrIPDriver(Backend &backend)
  : CanDriver(backend),
    setupPage(new GenericCanSetupPage())
{
    QObject::connect(&backend, SIGNAL(onSetupDialogCreated(SetupDialog&)), setupPage, SLOT(onSetupDialogCreated(SetupDialog&)));
    m_GrIPHandler = nullptr;
}

GrIPDriver::~GrIPDriver()
{
    if(m_GrIPHandler)
        delete m_GrIPHandler;
}

bool GrIPDriver::update()
{
    deleteAllInterfaces();

    int interface_cnt = 0;

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // fprintf(stderr, "Name : %s \r\n",  info.portName().toStdString().c_str());
        // fprintf(stderr, "   Description : %s \r\n", info.description().toStdString().c_str());
        // fprintf(stderr, "   Manufacturer: %s \r\n", info.manufacturer().toStdString().c_str());

        if(info.vendorIdentifier() == 0x0403 && info.productIdentifier() == 0x6015)
        {
            std::cout << "   ++ CANIL detected" << std::endl;

            if(m_GrIPHandler == nullptr)
            {
                m_GrIPHandler = new GrIPHandler(info.portName());

                if(!m_GrIPHandler->Start())
                {
                }

                m_GrIPHandler->RequestVersion();
                QThread().msleep(15);
            }

            // Create new CANIL interface
            _manufacturer = GrIPInterface::CANIL;
            for(int i = 0; i < m_GrIPHandler->Channels_CAN(); i++)
            {
                createOrUpdateInterface(interface_cnt, m_GrIPHandler, "CANIL-CAN"+QString::number(interface_cnt), false, _manufacturer);
                interface_cnt++;
            }
            // Create new CANIL interface wit FD support
            for(int i = 0; i < m_GrIPHandler->Channels_CANFD(); i++)
            {
                createOrUpdateInterface(interface_cnt, m_GrIPHandler, "CANIL-CANFD"+QString::number(interface_cnt), true, _manufacturer);
                interface_cnt++;
            }
        }
        else
        {
            //std::cout << "   !! This is not a GrIP device!" << std::endl;
        }
    }

    return true;
}

QString GrIPDriver::getName()
{
    return "GrIP-CANIL";
}

GrIPInterface *GrIPDriver::createOrUpdateInterface(int index, GrIPHandler *hdl, QString name, bool fd_support, uint32_t manufacturer)
{
    foreach (CanInterface *intf, getInterfaces())
    {
        GrIPInterface *scif = dynamic_cast<GrIPInterface*>(intf);
        if (scif->getIfIndex() == index)
        {
            scif->setName(name);
            return scif;
		}
	}

    GrIPInterface *scif = new GrIPInterface(this, index, hdl, name, fd_support, manufacturer);
    addInterface(scif);

    return scif;
}
