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

#include "CanDbSignal.h"

CanDbSignal::CanDbSignal(CanDbMessage *parent)
  : _parent(parent),
    _isUnsigned(false),
    _factor(1),
    _offset(0),
    _min(0),
    _max(0)
{
}

QString CanDbSignal::name() const
{
    return _name;
}

void CanDbSignal::setName(const QString &name)
{
    _name = name;
}

int CanDbSignal::startBit() const
{
    return _startBit;
}

void CanDbSignal::setStartBit(int startBit)
{
    _startBit = startBit;
}

int CanDbSignal::length() const
{
    return _length;
}

void CanDbSignal::setLength(int length)
{
    _length = length;
}

QString CanDbSignal::comment() const
{
    return _comment;
}

void CanDbSignal::setComment(const QString &comment)
{
    _comment = comment;
}

QString CanDbSignal::getValueName(const uint32_t value) const
{
    if (_valueTable.contains(value)) {
        return _valueTable[value];
    } else {
        return QString();
    }
}

void CanDbSignal::setValueName(const uint32_t value, const QString &name)
{
    _valueTable[value] = name;
}

double CanDbSignal::convertRawValueToPhysical(const uint32_t rawValue)
{
    int v;
    if (isUnsigned()) {
        v = rawValue;
    } else {
        // TODO check with DBC that actually contains signed values?!
        v = (int32_t)(rawValue<<(32-_length));
        v>>=(32-_length);
    }
    return v * _factor + _offset;
}

double CanDbSignal::getFactor() const
{
    return _factor;
}

void CanDbSignal::setFactor(double factor)
{
    _factor = factor;
}

double CanDbSignal::getOffset() const
{
    return _offset;
}

void CanDbSignal::setOffset(double offset)
{
    _offset = offset;
}

double CanDbSignal::getMinimumValue() const
{
    return _min;
}

void CanDbSignal::setMinimumValue(double min)
{
    _min = min;
}

double CanDbSignal::getMaximumValue() const
{
    return _max;
}

void CanDbSignal::setMaximumValue(double max)
{
    _max = max;
}

bool CanDbSignal::isUnsigned() const
{
    return _isUnsigned;
}

void CanDbSignal::setUnsigned(bool isUnsigned)
{
    _isUnsigned = isUnsigned;
}

