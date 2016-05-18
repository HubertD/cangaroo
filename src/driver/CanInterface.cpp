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

QString CanInterface::getDetailsStr() const
{
    return "";
}

uint32_t CanInterface::getCapabilities()
{
    return 0;
}

QList<CanTiming> CanInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;
    retval << CanTiming(0,   10000, 0, 875) \
           << CanTiming(1,   20000, 0, 875) \
           << CanTiming(2,   50000, 0, 875) \
           << CanTiming(3,   83333, 0, 875) \
           << CanTiming(4,  100000, 0, 875) \
           << CanTiming(5,  125000, 0, 875) \
           << CanTiming(6,  250000, 0, 875) \
           << CanTiming(7,  500000, 0, 875) \
           << CanTiming(8,  800000, 0, 875) \
           << CanTiming(9, 1000000, 0, 875);
    return retval;
}

void CanInterface::open() {
}

void CanInterface::close() {
}

bool CanInterface::updateStatistics()
{
    return false;
}

QString CanInterface::getStateText()
{
    switch (getState()) {
        case state_ok: return "ok";
        case state_warning: return "warning";
        case state_passive: return "error passive";
        case state_bus_off: return "bus off";
        case state_stopped: return "stopped";
        case state_unknown: return "unknown";
        default: return "";
    }
}

CanInterfaceId CanInterface::getId() const
{
    return _id;
}

void CanInterface::setId(CanInterfaceId id)
{
    _id = id;
}
