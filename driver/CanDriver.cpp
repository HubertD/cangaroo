/*
 * CanDriver.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "CanDriver.h"

CanDriver::CanDriver()
{
    setId(0);
}

CanDriver::~CanDriver() {
    // TODO Auto-generated destructor stub
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
