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

#include <stdint.h>
#include <QString>
#include "CanDb.h"
#include "CanDbSignal.h"

class CanDbNode;
class CanDbSignal;

typedef QList<CanDbSignal*> CanDbSignalList;

class CanDbMessage
{
    public:
        CanDbMessage(CanDb *parent);

        QString getName() const;
        void setName(const QString &name);

        uint32_t getRaw_id() const;
        void setRaw_id(const uint32_t &raw_id);

        uint8_t getDlc() const;
        void setDlc(const uint8_t &dlc);

        CanDbNode *getSender() const;
        void setSender(CanDbNode *sender);

        void addSignal(CanDbSignal *signal);
        CanDbSignal *getSignal(int num);
        CanDbSignal *getSignalByName(QString signal_name);

        CanDbSignalList getSignals() { return _signals; }

        QString getComment() const;
        void setComment(const QString &comment);

        CanDbSignal *getMuxer() const;
        void setMuxer(CanDbSignal *muxer);

private:
        CanDb *_parent;
        QString _name;
        uint32_t _raw_id;
        uint8_t _dlc;
        CanDbNode *_sender;
        CanDbSignalList _signals;
        QString _comment;
        CanDbSignal *_muxer;

};
