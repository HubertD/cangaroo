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

#ifndef CANDBSIGNAL_H
#define CANDBSIGNAL_H

#include "CanMessage.h"
#include "CanDbMessage.h"
#include <QString>
#include <QMap>

class CanDbMessage;

typedef QMap<uint32_t,QString> CanDbValueTable;

class CanDbSignal
{
public:
    CanDbSignal(CanDbMessage *parent);
    QString name() const;
    void setName(const QString &name);

    int startBit() const;
    void setStartBit(int startBit);

    int length() const;
    void setLength(int length);

    QString comment() const;
    void setComment(const QString &comment);

    QString getValueName(const uint32_t value) const;
    void setValueName(const uint32_t value, const QString &name);

    double convertRawValueToPhysical(const uint32_t rawValue);

    double getFactor() const;
    void setFactor(double factor);

    double getOffset() const;
    void setOffset(double offset);

    double getMinimumValue() const;
    void setMinimumValue(double min);

    double getMaximumValue() const;
    void setMaximumValue(double max);

    bool isUnsigned() const;
    void setUnsigned(bool isUnsigned);

    bool isBigEndian() const;
    void setIsBigEndian(bool isBigEndian);

private:
    CanDbMessage *_parent;
    QString _name;
    int _startBit;
    int _length;
    bool _isUnsigned;
    bool _isBigEndian;
    double _factor;
    double _offset;
    double _min;
    double _max;
    QString _comment;
    CanDbValueTable _valueTable;
};

#endif // CANDBSIGNAL_H
