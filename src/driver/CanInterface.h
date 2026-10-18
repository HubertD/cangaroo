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
#include <sys/time.h>
#include <atomic>
#include "CanDriver.h"
#include "CanTiming.h"
#include <QObject>

class CanMessage;
class MeasurementInterface;

class CanInterface: public QObject  {
    Q_OBJECT
public:
    enum {
        state_ok,
        state_warning,
        state_passive,
        state_bus_off,
        state_stopped,
        state_unknown,
        state_tx_success,
        state_tx_fail,
    };

    enum {
        capability_canfd           = 0x01,
        capability_listen_only     = 0x02,
        capability_triple_sampling = 0x04,
        capability_one_shot        = 0x08,
        capability_auto_restart    = 0x10,
        capability_config_os       = 0x20,
        capability_custom_bitrate  = 0x40,
        capability_custom_canfd_bitrate = 0x80
    };

public:
    CanInterface(CanDriver *driver);
	virtual ~CanInterface();
    virtual CanDriver *getDriver();
    virtual QString getName() const = 0;
    virtual QString getDetailsStr() const;

    virtual void applyConfig(const MeasurementInterface &mi) = 0;

    virtual unsigned getBitrate() = 0;

    virtual uint32_t getCapabilities();
    virtual QList<CanTiming> getAvailableBitrates();

	virtual void open();
	virtual void close();

    virtual bool isOpen();

    virtual void sendMessage(const CanMessage &msg) = 0;
    virtual bool readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms) = 0;

    virtual bool updateStatistics();
    virtual void resetStatistics() { _totalBits = 0; }
    virtual uint32_t getState() = 0;
    virtual int getNumRxFrames() = 0;
    virtual int getNumRxErrors() = 0;
    virtual int getNumTxFrames() = 0;
    virtual int getNumTxErrors() = 0;
    virtual int getNumRxOverruns() = 0;
    virtual int getNumTxDropped() = 0;
    virtual uint64_t getNumBits();
    void addFrameBits(const CanMessage &msg);

    virtual QString getVersion();

    QString getStateText();

    CanInterfaceId getId() const;
    void setId(CanInterfaceId id);

private:
    CanInterfaceId _id;
    CanDriver *_driver;
    std::atomic<uint64_t> _totalBits;
};
