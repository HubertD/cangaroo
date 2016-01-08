/*
 * CanInterface.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "CanInterface.h"

#include <QList>

CanInterface::CanInterface(CanDriver *driver)
  : _id(-1), _driver(driver)
{
}

CanInterface::~CanInterface() {
}

CanDriver* CanInterface::getDriver() {
    return _driver;
}

QList<int> CanInterface::getAvailableBitrates()
{
    QList<int> retval;
    retval << 88300 << 100000 << 125000 << 250000 << 500000 << 1000000;
    return retval;
}

void CanInterface::open() {
}

void CanInterface::close() {
}

uint8_t CanInterface::getId()
{
    return _id;
}

void CanInterface::setId(uint8_t id)
{
    _id = id;
}
