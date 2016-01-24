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

#include <QString>
#include <stdint.h>

class CanDriver;
class CanMessage;
class MeasurementInterface;

class CanInterface {
public:
    enum {
        state_ok,
        state_warning,
        state_passive,
        state_bus_off,
        state_stopped,
        state_unknown
    };

public:
    CanInterface(CanDriver *driver);
	virtual ~CanInterface();
    virtual CanDriver *getDriver();
    virtual QString getName() const = 0;

    virtual void applyConfig(const MeasurementInterface &mi) = 0;

    virtual int getBitrate() = 0;

    virtual QList<int> getAvailableBitrates();
    virtual QList<int> getAvailableFdBitrates();

	virtual void open();
	virtual void close();

    virtual void sendMessage(const CanMessage &msg) = 0;
    virtual bool readMessage(CanMessage &msg, unsigned int timeout_ms) = 0;

    virtual bool updateStatistics();
    virtual uint32_t getState() = 0;
    virtual int getNumRxFrames() = 0;
    virtual int getNumRxErrors() = 0;
    virtual int getNumTxFrames() = 0;
    virtual int getNumTxErrors() = 0;
    virtual int getNumRxOverruns() = 0;
    virtual int getNumTxDropped() = 0;

    QString getStateText();

    uint8_t getId();
    void setId(uint8_t id);

private:
    uint8_t _id;
    CanDriver *_driver;
};
