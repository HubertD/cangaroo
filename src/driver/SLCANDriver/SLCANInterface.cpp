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

#include "SLCANInterface.h"
#include "qapplication.h"
#include "qdebug.h"

#include <core/Backend.h>
#include <core/MeasurementInterface.h>
#include <core/CanMessage.h>

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QThread>


SLCANInterface::SLCANInterface(SLCANDriver *driver, int index, QString name, bool fd_support, uint32_t manufacturer)
  : CanInterface((CanDriver *)driver),
    _manufacturer(manufacturer),
    _idx(index),
    _isOpen(false),
    _isOffline(false),
    _serport(NULL),
    _name(name),
    _rx_linbuf_ctr(0),
    _rxbuf_head(0),
    _rxbuf_tail(0),
    _ts_mode(ts_mode_SIOCSHWTSTAMP),
    _send_wait_respond(0)
{
    // Set defaults
    _settings.setBitrate(500000);
    _settings.setSamplePoint(875);

    _config.supports_canfd = fd_support;
    _config.supports_timing = false;

    if(fd_support)
    {
        _settings.setFdBitrate(2000000);
        _settings.setFdSamplePoint(750);
    }

    _status.can_state = state_bus_off;
    _status.rx_count = 0;
    _status.rx_errors = 0;
    _status.rx_overruns = 0;
    _status.tx_count = 0;
    _status.tx_errors = 0;
    _status.tx_dropped = 0;

    _readMessage_datetime = QDateTime::currentDateTime();

    _readMessage_datetime_run = QDateTime::currentDateTime();

    _can_msg_queue.clear();
    _can_msg_tx_queue.clear();
}

SLCANInterface::~SLCANInterface()
{
}

QString SLCANInterface::getDetailsStr() const
{
    if(_manufacturer == CANable)
    {
        if(_config.supports_canfd)
        {
            return tr("CANable with CANFD support");
        }
        else
        {
            return tr("CANable with standard CAN support");
        }
    }
    else if(_manufacturer == WeActStudio)
    {
        if(_config.supports_canfd)
        {
            return tr("WeAct Studio USB2CAN with CANFD support");
        }
        else
        {
            return tr("WeAct Studio USB2CAN with standard CAN support");
        }
    }
    else
    {
        return tr("Not Supported");
    }
}

QString SLCANInterface::getName() const
{
	return _name;
}

void SLCANInterface::setName(QString name)
{
    _name = name;
}

QList<CanTiming> SLCANInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;
    QList<unsigned> bitrates;
    QList<unsigned> bitrates_fd;

    QList<unsigned> samplePoints;
    QList<unsigned> samplePoints_fd;

    if(_manufacturer == CANable)
    {
        bitrates.append({10000, 20000, 50000, 83333, 100000, 125000, 250000, 500000, 800000, 1000000});
        bitrates_fd.append({2000000, 5000000});
        samplePoints.append({875});
        samplePoints_fd.append({750});
    }
    else if(_manufacturer == WeActStudio)
    {
        bitrates.append({5000, 10000, 20000, 33333, 50000, 62500, 75000, 83333, 100000, 125000, 250000, 500000, 800000, 1000000});
        bitrates_fd.append({1000000, 2000000, 3000000, 4000000, 5000000});
        samplePoints.append({875});
        samplePoints_fd.append({750});
    }

    unsigned i=0;
    foreach (unsigned br, bitrates)
    {
        for (unsigned br_fd : bitrates_fd)
        {
            foreach (unsigned sp, samplePoints)
            {
                foreach (unsigned sp_fd, samplePoints_fd)
                {
                    retval << CanTiming(i++, br, br_fd, sp,sp_fd);
                }
            }
        }
    }

    return retval;
}

void SLCANInterface::applyConfig(const MeasurementInterface &mi)
{
    // Save settings for port configuration
    _settings = mi;
}

bool SLCANInterface::updateStatus()
{
    return false;
}

bool SLCANInterface::readConfig()
{
    return false;
}

bool SLCANInterface::readConfigFromLink(rtnl_link *link)
{
    Q_UNUSED(link);
    return false;
}

bool SLCANInterface::supportsTimingConfiguration()
{
    return _config.supports_timing;
}

bool SLCANInterface::supportsCanFD()
{
    return _config.supports_canfd;
}

bool SLCANInterface::supportsTripleSampling()
{
    return false;
}

unsigned SLCANInterface::getBitrate()
{
    return _settings.bitrate();
}

uint32_t SLCANInterface::getCapabilities()
{
    uint32_t retval = 0;

    if(_manufacturer == CANable)
    {
        retval =
            CanInterface::capability_config_os |
            CanInterface::capability_auto_restart |
            CanInterface::capability_listen_only;
    }
    else if(_manufacturer == WeActStudio)
    {
        retval =
            // CanInterface::capability_config_os |
            // CanInterface::capability_auto_restart |
            CanInterface::capability_listen_only |
            CanInterface::capability_custom_bitrate |
            CanInterface::capability_custom_canfd_bitrate;
    }

    if (supportsCanFD())
    {
        retval |= CanInterface::capability_canfd;
    }

    if (supportsTripleSampling())
    {
        retval |= CanInterface::capability_triple_sampling;
    }

    return retval;
}

bool SLCANInterface::updateStatistics()
{
    return updateStatus();
}

uint32_t SLCANInterface::getState()
{
    return _status.can_state;
}

int SLCANInterface::getNumRxFrames()
{
    return _status.rx_count;
}

int SLCANInterface::getNumRxErrors()
{
    return _status.rx_errors;
}

int SLCANInterface::getNumTxFrames()
{
    return _status.tx_count;
}

int SLCANInterface::getNumTxErrors()
{
    return _status.tx_errors;
}

int SLCANInterface::getNumRxOverruns()
{
    return _status.rx_overruns;
}

int SLCANInterface::getNumTxDropped()
{
    return _status.tx_dropped;
}

int SLCANInterface::getIfIndex()
{
    return _idx;
}

QString SLCANInterface::getVersion()
{
    return _version;
}

void SLCANInterface::open()
{
    if(_serport != NULL)
    {
        delete _serport;
    }

    _serport = new QSerialPort();

    _serport_mutex.lock();
    _serport->setPortName(_name);
    _serport->setBaudRate(1000000);
    _serport->setDataBits(QSerialPort::Data8);
    _serport->setParity(QSerialPort::NoParity);
    _serport->setStopBits(QSerialPort::OneStop);
    _serport->setFlowControl(QSerialPort::NoFlowControl);
    _serport->setReadBufferSize(2048);

    if (_serport->open(QIODevice::ReadWrite))
    {
        //perror("Serport connected!");
        qRegisterMetaType<QSerialPort::SerialPortError>("SerialThread");
        //connect(_serport, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),  this, &SLCANInterface::handleSerialError);
        //connect(_serport, SIGNAL(readyRead()),this,SLOT(serport_readyRead()));
    }
    else
    {
        perror("Serport connect failed!");
        _serport_mutex.unlock();
        _isOpen = false;
        _isOffline = true;
        return;
    }
    _serport->flush();
    _serport->clear();

    // Close CAN port
    _serport->write("C\r", 2);
    _serport->flush();
    _serport->waitForBytesWritten(100);
    _serport->waitForReadyRead(50);

    // Get Version
    _serport->clear(QSerialPort::Input);
    _serport->write("V\r", 2);
    _serport->flush();
    _serport->waitForBytesWritten(100);
    if(_serport->waitForReadyRead(50))
    {
        qApp->processEvents();

        if(_serport->bytesAvailable())
        {
            // This is called when readyRead() is emitted
            QByteArray datas = _serport->readLine();
            _version = QString(datas).trimmed();
        }
    }

    if(_settings.isCustomBitrate())
    {
        QString _custombitrate = QString("%1").arg(_settings.customBitrate(), 6, 16,QLatin1Char('0')).toUpper();
        std::string _custombitrate_std= 'S' + _custombitrate.toStdString() + '\r';
        _serport->write(_custombitrate_std.c_str(), _custombitrate_std.length());
        _serport->flush();
    }
    else
    {
        // Set the classic CAN bitrate
        switch(_settings.bitrate())
        {
            case 1000000:
                _serport->write("S8\r", 3);
                _serport->flush();
                break;
            case 800000:
                _serport->write("S7\r", 3);
                _serport->flush();
                break;
            case 500000:
                _serport->write("S6\r", 3);
                _serport->flush();
                break;
            case 250000:
                _serport->write("S5\r", 3);
                _serport->flush();
                break;
            case 125000:
                _serport->write("S4\r", 3);
                _serport->flush();
                break;
            case 100000:
                _serport->write("S3\r", 3);
                _serport->flush();
                break;
            case 83333:
                _serport->write("S9\r", 3);
                _serport->flush();
                break;
            case 75000:
                _serport->write("SA\r", 3);
                _serport->flush();
                break;
            case 62500:
                _serport->write("SB\r", 3);
                _serport->flush();
                break;
            case 50000:
                _serport->write("S2\r", 3);
                _serport->flush();
                break;
            case 33333:
                _serport->write("SC\r", 3);
                _serport->flush();
                break;
            case 20000:
                _serport->write("S1\r", 3);
                _serport->flush();
                break;
            case 10000:
                _serport->write("S0\r", 3);
                _serport->flush();
                break;
            case 5000:
                _serport->write("SD\r", 3);
                _serport->flush();
                break;
            default:
                // Default to 10k
                _serport->write("S0\r", 3);
                _serport->flush();
                break;
        }
    }

    _serport->waitForBytesWritten(200);

    // Set configured BRS rate
    if(_config.supports_canfd)
    {
        if(_settings.isCustomFdBitrate())
        {
            QString _customfdbitrate = QString("%1").arg(_settings.customFdBitrate(), 6, 16,QLatin1Char('0')).toUpper();
            std::string _customfdbitrate_std= 'Y' + _customfdbitrate.toStdString() + '\r';
            _serport->write(_customfdbitrate_std.c_str(), _customfdbitrate_std.length());
            _serport->flush();
        }
        else
        {
            switch(_settings.fdBitrate())
            {
                case 1000000:
                    _serport->write("Y1\r", 3);
                    _serport->flush();
                    break;
                case 2000000:
                    _serport->write("Y2\r", 3);
                    _serport->flush();
                    break;
                case 3000000:
                    _serport->write("Y3\r", 3);
                    _serport->flush();
                    break;
                case 4000000:
                    _serport->write("Y4\r", 3);
                    _serport->flush();
                    break;
                case 5000000:
                    _serport->write("Y5\r", 3);
                    _serport->flush();
                    break;
            }
        }
    }
    _serport->waitForBytesWritten(100);

    // Set Listen Only Mode
    if(_settings.isListenOnlyMode())
    {
        _serport->write("M1\r", 3);
        _serport->flush();
    }
    else
    {
        _serport->write("M0\r", 3);
        _serport->flush();
    }
    _serport->waitForBytesWritten(100);

    // Open the port
    _serport->write("O\r", 2);
    _serport->flush();
    _serport->waitForBytesWritten(100);

    // Clear serial port receiver
    if(_serport->waitForReadyRead(10))
    {
        qApp->processEvents();

        if(_serport->bytesAvailable())
        {
            // This is called when readyRead() is emitted
            _serport->readAll();
        }
    }

    _can_msg_queue.clear();
    _can_msg_tx_queue.clear();
    _send_wait_respond = 0;
    memset(_rxbuf,0,sizeof(_rxbuf));
    memset(_rx_linbuf,0,sizeof(_rx_linbuf));

    _isOpen = true;
    _isOffline = false;
    _status.can_state = state_ok;
    _status.rx_count = 0;
    _status.rx_errors = 0;
    _status.rx_overruns = 0;
    _status.tx_count = 0;
    _status.tx_errors = 0;
    _status.tx_dropped = 0;

    // Release port mutex
    _serport_mutex.unlock();
}

void SLCANInterface::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError)
    {
        perror("error");

        _isOffline = true;
    }

    QString  ERRORString = "";
    switch (error) {
    case QSerialPort::NoError:
        ERRORString=  "No Error";
        break;
    case QSerialPort::DeviceNotFoundError:
        ERRORString= "Device Not Found";
        break;
    case QSerialPort::PermissionError:
        ERRORString= "Permission Denied";
        break;
    case QSerialPort::OpenError:
        ERRORString= "Open Error";
        break;
    /*case QSerialPort::ParityError:
        ERRORString= "Parity Error";
        break;
    case QSerialPort::FramingError:
        ERRORString= "Framing Error";
        break;
    case QSerialPort::BreakConditionError:
        ERRORString= "Break Condition";
        break;
    case QSerialPort::WriteError:
        ERRORString= "Write Error";
        break;*/
    case QSerialPort::ReadError:
        ERRORString= "Read Error";
        break;
    case QSerialPort::ResourceError:
        ERRORString= "Resource Error";
        break;
    case QSerialPort::UnsupportedOperationError:
        ERRORString= "Unsupported Operation";
        break;
    case QSerialPort::UnknownError:
        ERRORString= "Unknown Error";
        break;
    case QSerialPort::TimeoutError:
        //ERRORString= "Timeout Error";
        break;
    case QSerialPort::NotOpenError:
        ERRORString= "Not Open Error";
        break;
    default:
        ERRORString= "Other Error";
    }
    if(ERRORString.size())
        std::cout << "SerialPortWorker::errorOccurred  ,info is  " << ERRORString.toStdString() << std::endl;
}

void SLCANInterface::close()
{
    _serport_mutex.lock();

    _isOpen = false;
    _status.can_state = state_bus_off;

    if (_serport->isOpen())
    {
        // Close CAN port
        _serport->write("C\r", 2);
        _serport->flush();
        _serport->waitForBytesWritten(100);
        _serport->waitForReadyRead(10);
        _serport->clear();
        _serport->close();
    }

    _can_msg_queue.clear();
    _can_msg_tx_queue.clear();

    _serport_mutex.unlock();
}

bool SLCANInterface::isOpen()
{
    return _isOpen;
}

void SLCANInterface::sendMessage(const CanMessage &msg)
{
    _serport_mutex.lock();
    // SLCAN_MTU plus null terminator
    can_msg_t can_msg;

    uint8_t msg_idx = 0;

    // Message is FD
    // Add character for frame type
    if(msg.isFD())
    {
        if(msg.isBRS())
        {
            can_msg.buf[msg_idx] = 'b';

        }
        else
        {
            can_msg.buf[msg_idx] = 'd';
        }
    }
    else
    {
        // Message is not FD
        // Add character for frame type

        if (msg.isRTR()) {
            can_msg.buf[msg_idx] = 'r';
        }
        else
        {
            can_msg.buf[msg_idx] = 't';
        }
    }

    // Assume standard identifier
    uint8_t id_len = SLCAN_STD_ID_LEN;
    uint32_t tmp = msg.getId();

    // Check if extended
    if (msg.isExtended())
    {
        // Convert first char to upper case for extended frame
        can_msg.buf[msg_idx] -= 32;
        id_len = SLCAN_EXT_ID_LEN;
    }
    msg_idx++;

    // Add identifier to buffer
    for(uint8_t j = id_len; j > 0; j--)
    {
        // Add nibble to buffer
        can_msg.buf[j] = (tmp & 0xF);
        tmp = tmp >> 4;
        msg_idx++;
    }

    // Sanity check length
    int8_t bytes = msg.getLength();

    if(bytes < 0)
        return;
    if(bytes > 64)
        return;

    // If canfd
    if(bytes > 8)
    {
        switch(bytes)
        {
        case 12:
            bytes = 0x9;
            break;
        case 16:
            bytes = 0xA;
            break;
        case 20:
            bytes = 0xB;
            break;
        case 24:
            bytes = 0xC;
            break;
        case 32:
            bytes = 0xD;
            break;
        case 48:
            bytes = 0xE;
            break;
        case 64:
            bytes = 0xF;
            break;
        }
    }

    // Add DLC to buffer
    can_msg.buf[msg_idx++] = bytes;

    // Add data bytes
    for (uint8_t j = 0; j < msg.getLength(); j++)
    {
        can_msg.buf[msg_idx++] = (msg.getByte(j) >> 4);
        can_msg.buf[msg_idx++] = (msg.getByte(j) & 0x0F);
    }

    // Convert to ASCII (2nd character to end)
    for (uint8_t j = 1; j < msg_idx; j++)
    {
        if (can_msg.buf[j] < 0xA) {
            can_msg.buf[j] += 0x30;
        } else {
            can_msg.buf[j] += 0x37;
        }
    }

    // Add CR for slcan EOL
    can_msg.buf[msg_idx++] = '\r';

    // Ensure null termination
    can_msg.buf[msg_idx] = '\0';

    can_msg.length = msg_idx;

    _can_msg_queue.append(can_msg);
    _can_msg_tx_queue.append(msg);

    _serport_mutex.unlock();
}

bool SLCANInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms)
{
    CanMessage msgtx;
    QDateTime datetime;

    Q_UNUSED(timeout_ms);

    datetime = QDateTime::currentDateTime();
    if(datetime.toMSecsSinceEpoch() - _readMessage_datetime_run.toMSecsSinceEpoch() >= 1)
    {
        _readMessage_datetime_run = QDateTime::currentDateTime().addMSecs(1);
    }
    else
    {
        return false;
    }

    // Don't saturate the thread. Read the buffer every 1ms.
    QThread().msleep(1);

    if(_isOffline == true)
    {
        if(_isOpen)
            close();
        return false;
    }
    else
    {
        datetime = QDateTime::currentDateTime();
        if(datetime.toMSecsSinceEpoch() - _readMessage_datetime.toMSecsSinceEpoch() > 3000)
        {
            _status.can_state = state_ok;
            _send_wait_respond = 0;
        }
    }

    // Transmit all items that are queued
    can_msg_t tmp;

    QList<can_msg_t>::iterator it;
    _serport_mutex.lock();
    for(it = _can_msg_queue.begin(); it<_can_msg_queue.end();it++)
    {
        if(_can_msg_queue.empty())
        {
            std::cout << "msg empty1" << std::endl;
            break;
        }

        // Consume first item
        tmp = _can_msg_queue.front();
        _can_msg_queue.pop_front();

        // Write string to serial device
        if(_serport->write(tmp.buf, tmp.length)==tmp.length)
        {
            _send_wait_respond ++;
            _readMessage_datetime = QDateTime::currentDateTime();

            // if(_can_msg_tx_queue.empty() == false)
            // {
            //     msgtx.cloneFrom(_can_msg_tx_queue.front());
            //     if(_can_msg_tx_queue.empty() == false)
            //         _can_msg_tx_queue.pop_front();
            //     struct timeval tv;
            //     gettimeofday(&tv,NULL);
            //     msgtx.setTimestamp(tv);
            //     _can_msg_tx_queue.push_front(msgtx);

            // }
        }
        else
        {
            _status.tx_errors ++;
            //_send_wait_respond = 0;

            if(_can_msg_tx_queue.empty() == false)
            {
                _can_msg_tx_queue.pop_front();
            }
        }

        //_serport->flush();
        _serport->waitForBytesWritten(200);

        if(it >= _can_msg_queue.end())
        {
            std::cout << "msg empty" << std::endl;
            //_can_msg_queue.clear();
            break;
        }
    }
    _serport_mutex.unlock();

    // RX doesn't work on windows unless we call this for some reason
    _rxbuf_mutex.lock();
    if(_serport->waitForReadyRead(0))
    {
        qApp->processEvents();

        if(_serport->bytesAvailable())
        {
            // This is called when readyRead() is emitted
            QByteArray datas = _serport->readAll();

            for(int i=0; i<datas.size(); i++)
            {
                // If incrementing the head will hit the tail, we've filled the buffer. Reset and discard all data.
                if(((_rxbuf_head + 1) % RXCIRBUF_LEN) == _rxbuf_tail)
                {
                    _rxbuf_head = 0;
                    _rxbuf_tail = 0;
                }
                else
                {
                    // Put inbound data at the head locatoin
                    _rxbuf[_rxbuf_head] = datas.at(i);
                    _rxbuf_head = (_rxbuf_head + 1) % RXCIRBUF_LEN; // Wrap at MTU
                }
            }

        }
    }
    _rxbuf_mutex.unlock();

    //////////////////////////

    bool ret = true;
    _rxbuf_mutex.lock();
    while(_rxbuf_tail != _rxbuf_head)
    {
        // Save data if room
        if(_rx_linbuf_ctr <= SLCAN_MTU)
        {
            _rx_linbuf[_rx_linbuf_ctr] = _rxbuf[_rxbuf_tail];
            _rx_linbuf_ctr++;
            // std::cout << "result " << std::hex << int(_rxbuf[_rxbuf_tail]) << std::endl;
            // std::cout << "_rxbuf_tail " << _rxbuf_tail << std::endl;
            // std::cout << "_rxbuf_head " << _rxbuf_head << std::endl;
            // std::cout << "_rx_linbuf_ctr " << _rx_linbuf_ctr << std::endl;
            // If we have a newline, then we just finished parsing a CAN message.
            if(_rxbuf[_rxbuf_tail] == '\r')
            {
                if(_rx_linbuf_ctr > 1)
                {
                    CanMessage msg;
                    ret = parseMessage(msg);
                    if(ret == true)
                    {
                         msglist.append(msg);
                        _status.rx_count ++;
                    }
                }
                else
                {
                    if(_send_wait_respond)
                    {
                        datetime = QDateTime::currentDateTime();
                        if(datetime.toMSecsSinceEpoch() - _readMessage_datetime.toMSecsSinceEpoch() < 200)
                        {
                            _status.tx_count ++;
                            _status.can_state = state_tx_success;
                        }
                        _send_wait_respond --;

                        if(_can_msg_tx_queue.empty() == false)
                        {
                            if(_status.can_state == state_tx_success)
                            {
                                msgtx.cloneFrom(_can_msg_tx_queue.front());
                                if(msgtx.isShow())
                                    msglist.append(msgtx);
                            }
                            if(_can_msg_tx_queue.empty() == false)
                                _can_msg_tx_queue.pop_front();
                        }
                    }
                }
                _rx_linbuf_ctr = 0;
            }
            else if(_rxbuf[_rxbuf_tail] == '\x07')
            {
                if(_rx_linbuf_ctr == 1)
                {
                    if(_send_wait_respond)
                    {
                        datetime = QDateTime::currentDateTime();
                        if(datetime.toMSecsSinceEpoch() - _readMessage_datetime.toMSecsSinceEpoch() < 200)
                        {
                            _status.tx_errors ++;
                            _status.can_state = state_tx_fail;
                        }
                        _send_wait_respond --;

                        if(_can_msg_tx_queue.empty() == false)
                            _can_msg_tx_queue.pop_front();
                    }
                }
                _rx_linbuf_ctr = 0;
            }
        }
        // Discard data if not
        else
        {
            perror("Linbuf full");
            _rx_linbuf_ctr = 0;
        }

        _rxbuf_tail = (_rxbuf_tail + 1) % RXCIRBUF_LEN;
    }
    _rxbuf_mutex.unlock();

    return ret;
}

bool SLCANInterface::parseMessage(CanMessage &msg)
{
    // Set timestamp to current time
    qint64 msec = QDateTime::currentMSecsSinceEpoch();
    msg.setTimestamp({
        static_cast<long>(msec / 1000),        // Sekunden
        static_cast<long>((msec % 1000) * 1000) // Mikrosekunden
    });

    // Defaults
    msg.setErrorFrame(0);
    msg.setInterfaceId(getId());
    msg.setId(0);
    msg.setRTR(false);
    msg.setFD(false);
    msg.setBRS(false);
    msg.setRX(true);

    // Convert from ASCII (2nd character to end)
    for (int i = 1; i < _rx_linbuf_ctr; i++)
    {
        // Lowercase letters
        if(_rx_linbuf[i] >= 'a')
            _rx_linbuf[i] = _rx_linbuf[i] - 'a' + 10;
        // Uppercase letters
        else if(_rx_linbuf[i] >= 'A')
            _rx_linbuf[i] = _rx_linbuf[i] - 'A' + 10;
        // Numbers
        else
            _rx_linbuf[i] = _rx_linbuf[i] - '0';
    }

    bool is_extended = false;
    bool is_rtr = false;

    // Handle each incoming command
    switch(_rx_linbuf[0])
    {
        // Transmit data frame command
        case 't':
        {
            is_extended = false;
        }
        break;
        case 'T':
        {
            is_extended = true;
        }
        break;

        // Transmit remote frame command
        case 'r':
        {
            is_extended = false;
            is_rtr = true;
        }
        break;
        case 'R':
        {
            is_extended = true;
            is_rtr = true;
        }
        break;

        // CANFD transmit - no BRS
        case 'd':
        {
            is_extended = false;
            msg.setFD(true);
            msg.setBRS(false);
        }
        break;
        case 'D':
        {
            is_extended = true;
            msg.setFD(true);
            msg.setBRS(false);
        }
        break;

        // CANFD transmit - with BRS
        case 'b':
        {
            is_extended = false;
            msg.setFD(true);
            msg.setBRS(true);
        }
        break;
        case 'B':
        {
            is_extended = true;
            msg.setFD(true);
            msg.setBRS(true);
        }
        break;

        // Invalid command
        default:
        {
            // Reset buffer
            _rx_linbuf_ctr = 0;
            _rx_linbuf[0] = '\0';
            return false;
        }
    }

    // Start parsing at second byte (skip command byte)
    uint8_t parse_loc = 1;

    // Default to standard id len
    uint8_t id_len = SLCAN_STD_ID_LEN;

    // Update length if message is extended ID
    if(is_extended)
        id_len = SLCAN_EXT_ID_LEN;

    uint32_t id_tmp = 0;

    // Iterate through ID bytes
    while(parse_loc <= id_len)
    {
        id_tmp <<= 4;
        id_tmp += _rx_linbuf[parse_loc++];
    }

    msg.setId(id_tmp);
    msg.setExtended(is_extended);
    msg.setRTR(is_rtr);

    // Attempt to parse DLC and check sanity
    uint8_t dlc_code_raw = _rx_linbuf[parse_loc++];

    // If dlc is too long for an FD frame
    if(msg.isFD() && dlc_code_raw > 0xF)
    {
        return false;
    }
    if(!msg.isFD() && dlc_code_raw > 0x8)
    {
        return false;
    }

    if(dlc_code_raw > 0x8)
    {
        switch(dlc_code_raw)
        {
        case 0x9:
            dlc_code_raw = 12;
            break;
        case 0xA:
            dlc_code_raw = 16;
            break;
        case 0xB:
            dlc_code_raw = 20;
            break;
        case 0xC:
            dlc_code_raw = 24;
            break;
        case 0xD:
            dlc_code_raw = 32;
            break;
        case 0xE:
            dlc_code_raw = 48;
            break;
        case 0xF:
            dlc_code_raw = 64;
            break;
        default:
            dlc_code_raw = 0;
            perror("Invalid length");
            break;
        }
    }

    msg.setLength(dlc_code_raw);

    // Calculate number of bytes we expect in the message
    int8_t bytes_in_msg = dlc_code_raw;

    if(bytes_in_msg < 0)
    {
        perror("Invalid length < 0");
        return false;
    }
    if(bytes_in_msg > 64)
    {
        perror("Invalid length > 64");
        return false;
    }

    // Parse data
    // TODO: Guard against walking off the end of the string!
    for (uint8_t i = 0; i < bytes_in_msg; i++)
    {
        msg.setByte(i,  (_rx_linbuf[parse_loc] << 4) + _rx_linbuf[parse_loc+1]);
        parse_loc += 2;
    }

    // Reset buffer
    _rx_linbuf_ctr = 0;
    _rx_linbuf[0] = '\0';

    return true;

/*
    // FIXME
    if (_ts_mode == ts_mode_SIOCSHWTSTAMP) {
        // TODO implement me
        _ts_mode = ts_mode_SIOCGSTAMPNS;
    }

    if (_ts_mode==ts_mode_SIOCGSTAMPNS) {
        if (ioctl(_fd, SIOCGSTAMPNS, &ts_rcv) == 0) {
            msg.setTimestamp(ts_rcv.tv_sec, ts_rcv.tv_nsec/1000);
        } else {
            _ts_mode = ts_mode_SIOCGSTAMP;
        }
    }

    if (_ts_mode==ts_mode_SIOCGSTAMP) {
        ioctl(_fd, SIOCGSTAMP, &tv_rcv);
        msg.setTimestamp(tv_rcv.tv_sec, tv_rcv.tv_usec);
    }*/
}
