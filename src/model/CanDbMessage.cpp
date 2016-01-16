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

#include "CanDbMessage.h"

CanDbMessage::CanDbMessage(CanDb *parent)
  : _parent(parent), _raw_id(0), _dlc(0), _sender(0), _muxer(0)
{
}

QString CanDbMessage::getName() const
{
    return _name;
}

void CanDbMessage::setName(const QString &name)
{
    _name = name;
}

uint32_t CanDbMessage::getRaw_id() const
{
    return _raw_id;
}

void CanDbMessage::setRaw_id(const uint32_t &raw_id)
{
    _raw_id = raw_id;
}

uint8_t CanDbMessage::getDlc() const
{
    return _dlc;
}

void CanDbMessage::setDlc(const uint8_t &dlc)
{
    _dlc = dlc;
}

CanDbNode *CanDbMessage::getSender() const
{
    return _sender;
}

void CanDbMessage::setSender(CanDbNode *sender)
{
    _sender = sender;
}

void CanDbMessage::addSignal(CanDbSignal *signal)
{
    _signals.append(signal);
}

CanDbSignal *CanDbMessage::getSignal(int num)
{
    if (_signals.size()>num) {
        return _signals[num];
    } else {
        return 0;
    }
}

CanDbSignal *CanDbMessage::getSignalByName(QString signal_name)
{
    CanDbSignal *signal;
    foreach (signal, _signals) {
        if (signal->name() == signal_name) {
            return signal;
        }
    }
    return 0;
}

QString CanDbMessage::getComment() const
{
    return _comment;
}

void CanDbMessage::setComment(const QString &comment)
{
    _comment = comment;
}

CanDbSignal *CanDbMessage::getMuxer() const
{
    return _muxer;
}

void CanDbMessage::setMuxer(CanDbSignal *muxer)
{
    _muxer = muxer;
}
