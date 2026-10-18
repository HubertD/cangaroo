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
    _isBigEndian(false),
    _factor(1),
    _offset(0),
    _min(0),
    _max(0),
    _isMuxer(false),
    _isMuxed(false),
    _muxValue(0)
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

uint8_t CanDbSignal::startBit() const
{
    return _startBit;
}

void CanDbSignal::setStartBit(uint8_t startBit)
{
    _startBit = startBit;
}

uint8_t CanDbSignal::length() const
{
    return _length;
}

void CanDbSignal::setLength(uint8_t length)
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

QString CanDbSignal::getValueName(const uint64_t value) const
{
    if (_valueTable.contains(value)) {
        return _valueTable[value];
    } else {
        return QString();
    }
}

void CanDbSignal::setValueName(const uint64_t value, const QString &name)
{
    _valueTable[value] = name;
}

double CanDbSignal::convertRawValueToPhysical(const uint64_t rawValue) const
{
    if (isUnsigned()) {
        uint64_t v = rawValue;
        return v * _factor + _offset;
    } else {
        // TODO check with DBC that actually contains signed values?!
        int64_t v = (int64_t)(rawValue<<(64-_length));
        v>>=(64-_length);
        return v * _factor + _offset;

    }
}

double CanDbSignal::extractPhysicalFromMessage(const CanMessage &msg) const
{
    return convertRawValueToPhysical(extractRawDataFromMessage(msg));
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

QString CanDbSignal::getUnit() const
{
    return _unit;
}

void CanDbSignal::setUnit(const QString &unit)
{
    _unit = unit;
}

bool CanDbSignal::isUnsigned() const
{
    return _isUnsigned;
}

void CanDbSignal::setUnsigned(bool isUnsigned)
{
    _isUnsigned = isUnsigned;
}
bool CanDbSignal::isBigEndian() const
{
    return _isBigEndian;
}

void CanDbSignal::setIsBigEndian(bool isBigEndian)
{
    _isBigEndian = isBigEndian;
}

bool CanDbSignal::isMuxer() const
{
    return _isMuxer;
}

void CanDbSignal::setIsMuxer(bool isMuxer)
{
    _isMuxer = isMuxer;
}

bool CanDbSignal::isMuxed() const
{
    return _isMuxed;
}

void CanDbSignal::setIsMuxed(bool isMuxed)
{
    _isMuxed = isMuxed;
}

uint32_t CanDbSignal::getMuxValue() const
{
    return _muxValue;
}

void CanDbSignal::setMuxValue(const uint32_t &muxValue)
{
    _muxValue = muxValue;
}

bool CanDbSignal::isPresentInMessage(const CanMessage &msg) const
{
    if (msg.getRawId() != _parent->getRaw_id()) {
        return false;
    }

    uint32_t max_byte = 0;
    if (!_isBigEndian) {
        max_byte = (_startBit + _length - 1) / 8;
    } else {
        int bits_in_first_byte = (_startBit % 8) + 1;
        if (_length <= bits_in_first_byte) {
            max_byte = _startBit / 8;
        } else {
            max_byte = (_startBit / 8) + 1 + (_length - bits_in_first_byte - 1) / 8;
        }
    }

    if (max_byte >= msg.getLength()) {
        return false;
    }

    if (!_isMuxed) { return true; }

    CanDbSignal *muxer = _parent->getMuxer();
    if (!muxer) { return false; }

    return _muxValue == muxer->extractRawDataFromMessage(msg);
}

uint64_t CanDbSignal::extractRawDataFromMessage(const CanMessage &msg) const
{
    return msg.extractRawSignal(startBit(), length(), isBigEndian());
}

void CanDbSignal::injectRawDataToMessage(uint64_t rawValue, CanMessage &msg)
{
    msg.setRawSignal(startBit(), length(), isBigEndian(), rawValue);
}

void CanDbSignal::applyPhysicalToMessage(double physicalValue, CanMessage &msg)
{
    if (_factor == 0.0) {
        return;
    }

    const double rawD = (physicalValue - _offset) / _factor;
    uint64_t rawValue = 0;

    if (isUnsigned()) {
        const double rounded = rawD < 0.0 ? 0.0 : rawD + 0.5;
        rawValue = static_cast<uint64_t>(rounded);
        if (_length < 64) {
            const uint64_t maxRaw = (1ULL << _length) - 1;
            if (rawValue > maxRaw) {
                rawValue = maxRaw;
            }
        }
    } else {
        int64_t v = static_cast<int64_t>(rawD < 0.0 ? rawD - 0.5 : rawD + 0.5);
        uint64_t mask = 0xFFFFFFFFFFFFFFFFULL;
        if (_length < 64) {
            mask = (1ULL << _length) - 1;
        }
        rawValue = static_cast<uint64_t>(v) & mask;
    }
    injectRawDataToMessage(rawValue, msg);
}
