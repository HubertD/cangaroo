/*

  Copyright (c) 2015, 2016 Hubert Denkmair

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

#include "../CanInterface.h"
#include "qdatetime.h"
#include <core/MeasurementInterface.h>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMutex>

// Maximum rx buffer len
#define SLCAN_MTU (1 + 8 + 1 + 128 + 1) // canfd 64 frame plus \r plus some padding
#define SLCAN_STD_ID_LEN 3
#define SLCAN_EXT_ID_LEN 8

#define RXCIRBUF_LEN 8192 // Buffer for received serial data, serviced at 1ms intervals

class SLCANDriver;

typedef struct {
    bool supports_canfd;
    bool supports_timing;
    uint32_t state;
    uint32_t base_freq;
    uint32_t sample_point;
    uint32_t ctrl_mode;
    uint32_t restart_ms;
} can_config_t;

typedef struct {
    uint32_t can_state;

    uint64_t rx_count;
    int rx_errors;
    uint64_t rx_overruns;

    uint64_t tx_count;
    int tx_errors;
    uint64_t tx_dropped;
} can_status_t;

typedef struct {
    char buf[SLCAN_MTU+1];
    qint64 length;
} can_msg_t;

class SLCANInterface: public CanInterface {
    Q_OBJECT
public:
    enum {
        CANable,
        WeActStudio,
    };
public:
    SLCANInterface(SLCANDriver *driver, int index, QString name, bool fd_support, uint32_t manufacturer);
    virtual ~SLCANInterface();

    QString getDetailsStr() const;
    virtual QString getName() const;
    void setName(QString name);

    virtual QList<CanTiming> getAvailableBitrates();

    virtual void applyConfig(const MeasurementInterface &mi);
    virtual bool readConfig();
    virtual bool readConfigFromLink(struct rtnl_link *link);

    bool supportsTimingConfiguration();
    bool supportsCanFD();
    bool supportsTripleSampling();

    virtual unsigned getBitrate();
    virtual uint32_t getCapabilities();

	virtual void open();
    virtual void close();
    virtual bool isOpen();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms);

    virtual bool updateStatistics();
    virtual uint32_t getState();
    virtual int getNumRxFrames();
    virtual int getNumRxErrors();
    virtual int getNumRxOverruns();

    virtual int getNumTxFrames();
    virtual int getNumTxErrors();
    virtual int getNumTxDropped();

    virtual QString getVersion();

    int getIfIndex();

private:
    typedef enum {
        ts_mode_SIOCSHWTSTAMP,
        ts_mode_SIOCGSTAMPNS,
        ts_mode_SIOCGSTAMP
    } ts_mode_t;

    uint32_t _manufacturer;
    QString _version;

    int _idx;
    bool _isOpen;
    bool _isOffline;
    QSerialPort* _serport;
    QList<can_msg_t> _can_msg_queue;
    QList<CanMessage> _can_msg_tx_queue;
    QMutex _serport_mutex;
    QString _name;
    char _rx_linbuf[SLCAN_MTU+1];
    int _rx_linbuf_ctr;

    char _rxbuf[RXCIRBUF_LEN];
    uint32_t _rxbuf_head;
    uint32_t _rxbuf_tail;

    QMutex _rxbuf_mutex;
    MeasurementInterface _settings;

    can_config_t _config;
    can_status_t _status;
    ts_mode_t _ts_mode;

    QDateTime  _readMessage_datetime;
    uint32_t _send_wait_respond;
    QDateTime  _readMessage_datetime_run;

    bool updateStatus();
    bool parseMessage(CanMessage &msg);

private slots:
    void handleSerialError(QSerialPort::SerialPortError error);
};
