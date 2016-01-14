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

QList<int> CanInterface::getAvailableFdBitrates()
{
    QList<int> retval;
    retval << 1000000 << 2000000 << 4000000 << 8000000 << 16000000;
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
