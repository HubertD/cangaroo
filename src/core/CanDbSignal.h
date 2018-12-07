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

#pragma once

#include "CanMessage.h"
#include "CanDbMessage.h"
#include <QString>
#include <QMap>

class CanDbMessage;

typedef QMap<uint64_t,QString> CanDbValueTable;

class CanDbSignal
{
public:
    CanDbSignal(CanDbMessage *parent);
    QString name() const;
    void setName(const QString &name);

    uint8_t startBit() const;
    void setStartBit(uint8_t startBit);

    uint8_t length() const;
    void setLength(uint8_t length);

    QString comment() const;
    void setComment(const QString &comment);

    QString getValueName(const uint64_t value) const;
    void setValueName(const uint64_t value, const QString &name);

    double getFactor() const;
    void setFactor(double factor);

    double getOffset() const;
    void setOffset(double offset);

    double getMinimumValue() const;
    void setMinimumValue(double min);

    double getMaximumValue() const;
    void setMaximumValue(double max);

    QString getUnit() const;
    void setUnit(const QString &unit);

    bool isUnsigned() const;
    void setUnsigned(bool isUnsigned);

    bool isBigEndian() const;
    void setIsBigEndian(bool isBigEndian);

    bool isMuxer() const;
    void setIsMuxer(bool isMuxer);

    bool isMuxed() const;
    void setIsMuxed(bool isMuxed);

    uint32_t getMuxValue() const;
    void setMuxValue(const uint32_t &muxValue);

    bool isPresentInMessage(const CanMessage &msg);
    uint64_t extractRawDataFromMessage(const CanMessage &msg);

    double convertRawValueToPhysical(const uint64_t rawValue);
    double extractPhysicalFromMessage(const CanMessage &msg);


private:
    CanDbMessage *_parent;
    QString _name;
    uint8_t _startBit;
    uint8_t _length;
    bool _isUnsigned;
    bool _isBigEndian;
    double _factor;
    double _offset;
    double _min;
    double _max;
    QString _unit;
    bool _isMuxer;
    bool _isMuxed;
    uint32_t _muxValue;
    QString _comment;
    CanDbValueTable _valueTable;
};
