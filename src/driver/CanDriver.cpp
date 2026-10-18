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

#include "CanDriver.h"
#include <core/Backend.h>
#include <driver/CanInterface.h>

CanDriver::CanDriver(Backend &backend)
  : _backend(backend)
{
    setId(0);
}

CanDriver::~CanDriver() {
    // TODO Auto-generated destructor stub
}

Backend &CanDriver::backend() const
{
    return _backend;
}

void CanDriver::init(int id)
{
    setId(id);
}

void CanDriver::setId(int id)
{
    _id = id;
}

int CanDriver::id()
{
    return _id;
}

QList<CanInterface *> CanDriver::getInterfaces() const
{
    return _interfaces;
}

CanInterfaceIdList CanDriver::getInterfaceIds() const
{
    CanInterfaceIdList retval;
    foreach (CanInterface *intf, _interfaces) {
        retval.push_back(intf->getId());
    }
    return retval;
}

CanInterface *CanDriver::getInterfaceById(CanInterfaceId id)
{
    return _interfaces.value(id & 0xFF);
}

CanInterfaceId CanDriver::addInterface(CanInterface *intf)
{
    intf->setId((id()<<8) | _interfaces.size());
    _interfaces.push_back(intf);
    return intf->getId();
}

void CanDriver::deleteInterface(CanInterface *intf)
{
    delete intf;
    _interfaces.removeOne(intf);
}

void CanDriver::deleteAllInterfaces()
{
    for(CanInterface* interface: _interfaces)
    {
        delete interface;
        _interfaces.removeOne(interface);
    }
}



CanInterface *CanDriver::getInterfaceByName(QString ifName)
{
    foreach (CanInterface *intf, _interfaces) {
        if (intf->getName() == ifName) {
            return intf;
        }
    }
    return 0;
}
