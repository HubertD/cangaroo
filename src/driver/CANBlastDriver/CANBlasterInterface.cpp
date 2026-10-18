/*

  Copyright (c) 2022 Ethan Zonca

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

#include "CANBlasterInterface.h"

#include <core/Backend.h>
#include <core/MeasurementInterface.h>
#include <core/CanMessage.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include <QNetworkDatagram>


CANBlasterInterface::CANBlasterInterface(CANBlasterDriver *driver, int index, QString name, bool fd_support)
  : CanInterface((CanDriver *)driver),
	_idx(index),
    _isOpen(false),
    _name(name),
    _ts_mode(ts_mode_SIOCSHWTSTAMP),
    _socket(NULL)
{
    // Set defaults
    _settings.setBitrate(500000);
    _settings.setSamplePoint(875);

    _config.supports_canfd = fd_support;

    // Record start time
    _heartbeat_time_ms = QDateTime::currentMSecsSinceEpoch();
}

CANBlasterInterface::~CANBlasterInterface() {
}

QString CANBlasterInterface::getDetailsStr() const {
    if(_config.supports_canfd)
    {
        return tr("CANBlaster client with CANFD support");
    }
    else
    {
        return tr("CANBlaster client with standard CAN support");
    }
}

QString CANBlasterInterface::getName() const {
	return _name;
}

void CANBlasterInterface::setName(QString name) {
    _name = name;
}

QList<CanTiming> CANBlasterInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;
    QList<unsigned> bitrates({10000, 20000, 50000, 83333, 100000, 125000, 250000, 500000, 800000, 1000000});
    QList<unsigned> bitrates_fd({0, 2000000, 5000000});

    QList<unsigned> samplePoints({875});

    unsigned i=0;
    foreach (unsigned br, bitrates) {
        for (unsigned br_fd : bitrates_fd) {
            foreach (unsigned sp, samplePoints) {
                retval << CanTiming(i++, br, br_fd, sp);
            }
        }
    }

    return retval;
}


void CANBlasterInterface::applyConfig(const MeasurementInterface &mi)
{
    // Save settings for port configuration
    _settings = mi;
}



bool CANBlasterInterface::supportsTimingConfiguration()
{
    return _config.supports_timing;
}

bool CANBlasterInterface::supportsCanFD()
{
    return _config.supports_canfd;
}

bool CANBlasterInterface::supportsTripleSampling()
{
    return false;
}

unsigned CANBlasterInterface::getBitrate()
{
    return 0;
}

uint32_t CANBlasterInterface::getCapabilities()
{
    uint32_t retval =
        CanInterface::capability_config_os |
        CanInterface::capability_listen_only |
        CanInterface::capability_auto_restart;

    if (supportsCanFD()) {
        retval |= CanInterface::capability_canfd;
    }

    if (supportsTripleSampling()) {
        retval |= CanInterface::capability_triple_sampling;
    }

    return retval;
}

bool CANBlasterInterface::updateStatistics()
{
    return false;
}

uint32_t CANBlasterInterface::getState()
{
    if(_isOpen)
        return state_ok;
    else
        return state_bus_off;
}

int CANBlasterInterface::getNumRxFrames()
{
    return _status.rx_count;
}

int CANBlasterInterface::getNumRxErrors()
{
    return _status.rx_errors;
}

int CANBlasterInterface::getNumTxFrames()
{
    return _status.tx_count;
}

int CANBlasterInterface::getNumTxErrors()
{
    return _status.tx_errors;
}

int CANBlasterInterface::getNumRxOverruns()
{
    return _status.rx_overruns;
}

int CANBlasterInterface::getNumTxDropped()
{
    return _status.tx_dropped;
}

int CANBlasterInterface::getIfIndex() {
    return _idx;
}

const char *CANBlasterInterface::cname()
{
    return _name.toStdString().c_str();
}

void CANBlasterInterface::open()
{

    // Start off with a fresh socket
    if(_socket != NULL)
    {
        delete _socket;
    }
    _socket = new QUdpSocket();

    if(_socket->bind(QHostAddress::AnyIPv4, 20001))
    {
        _isOpen = true;
    }
    else
    {
        perror("CANBlaster Bind Failed!");
        _isOpen = false;
    }
}

void CANBlasterInterface::close()
{
    if(_socket != NULL && _socket->isOpen())
    {
        _socket->close();
    }
    _isOpen = false;
}

bool CANBlasterInterface::isOpen()
{
    return _isOpen;
}

void CANBlasterInterface::sendMessage(const CanMessage &msg)
{
    Q_UNUSED(msg);
}

bool CANBlasterInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms)
{
    // Don't saturate the thread
    QThread().usleep(250);

    Q_UNUSED(timeout_ms);

    // Record start time
    qint64 current_time_ms = QDateTime::currentMSecsSinceEpoch();

    if(current_time_ms - _heartbeat_time_ms > 1000)
    {

        _heartbeat_time_ms = current_time_ms;

        if(_isOpen)
        {
            QByteArray Data;
            Data.append("Heartbeat");
            _socket->writeDatagram(Data, QHostAddress(getName()), 20002);
        }
    }

    // NOTE: This only works with standard CAN frames right now!

    // Process all pending datagrams
    // TODO: Could switch to if from while but the caller expects 1 can frame
    if (_isOpen && _socket->hasPendingDatagrams())
    {
        can_frame frame;
        QHostAddress address;
        quint16 port;
        int res = _socket->readDatagram((char*)&frame, sizeof(can_frame), &address, &port);

        // TODO: Read all bytes... to CANFD_MTU max
//        if (nbytes == CANFD_MTU) {
//                printf("got CAN FD frame with length %d\n", cfd.len);
//                /* cfd.flags contains valid data */
//        } else if (nbytes == CAN_MTU) {
//                printf("got Classical CAN frame with length %d\n", cfd.len);
//                /* cfd.flags is undefined */
//        } else {
//                fprintf(stderr, "read: invalid CAN(FD) frame\n");
//                return 1;
//        }

        if(res > 0)
        {
            // Set timestamp to current time
            CanMessage msg;

            qint64 msec = QDateTime::currentMSecsSinceEpoch();
            msg.setTimestamp({
                static_cast<long>(msec / 1000),        // Sekunden
                static_cast<long>((msec % 1000) * 1000) // Mikrosekunden
            });

            msg.setInterfaceId(getId());
            msg.setId(frame.can_id & CAN_ERR_MASK);
            msg.setBRS(false);
            msg.setErrorFrame(frame.can_id & CAN_ERR_FLAG);
            msg.setExtended(frame.can_id & CAN_EFF_FLAG);
            msg.setRTR(frame.can_id & CAN_RTR_FLAG);
            msg.setLength(frame.len);

            for(int i=0; i<frame.len && i<CAN_MAX_DLEN; i++)
            {
                msg.setDataAt(i, frame.data[i]);
            }
            msglist.append(msg);
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;

}

