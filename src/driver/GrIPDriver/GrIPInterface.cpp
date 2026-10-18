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

#include "GrIPInterface.h"
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

#include "GrIP/GrIPHandler.h"


GrIPInterface::GrIPInterface(GrIPDriver *driver, int index, GrIPHandler *hdl, QString name, bool fd_support, uint32_t manufacturer)
  : CanInterface((CanDriver *)driver),
    _manufacturer(manufacturer),
    _idx(index),
    _isOpen(false),
    _isOffline(false),
    _serport(NULL),
    _name(name),
    _ts_mode(ts_mode_SIOCSHWTSTAMP),
    m_GrIPHandler(hdl)
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

    m_TxFrames.clear();
}

GrIPInterface::~GrIPInterface()
{
}

QString GrIPInterface::getDetailsStr() const
{
    if(_manufacturer == CANIL)
    {
        if(_config.supports_canfd)
        {
            return tr("CANIL with CANFD support");
        }
        else
        {
            return tr("CANIL with standard CAN support");
        }
    }
    else
    {
        return tr("Not Supported");
    }
}

QString GrIPInterface::getName() const
{
	return _name;
}

void GrIPInterface::setName(QString name)
{
    _name = name;
}

QList<CanTiming> GrIPInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;
    QList<unsigned> bitrates;
    QList<unsigned> bitrates_fd;

    QList<unsigned> samplePoints;
    QList<unsigned> samplePoints_fd;

    if(_manufacturer == GrIPInterface::CANIL)
    {
        bitrates.append({10000, 20000, 50000, 100000, 125000, 250000, 500000, 800000, 1000000});
        bitrates_fd.append({2000000, 5000000});
        samplePoints.append({875});
        samplePoints_fd.append({750});
    }
    /*else if(_manufacturer == WeActStudio)
    {
    }*/

    unsigned i = 0;
    foreach (unsigned br, bitrates)
    {
        foreach(unsigned br_fd, bitrates_fd)
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

void GrIPInterface::applyConfig(const MeasurementInterface &mi)
{
    // Save settings for port configuration
    _settings = mi;
}

bool GrIPInterface::updateStatus()
{
    return false;
}

bool GrIPInterface::readConfig()
{
    return false;
}

bool GrIPInterface::readConfigFromLink(rtnl_link *link)
{
    Q_UNUSED(link);
    return false;
}

bool GrIPInterface::supportsTimingConfiguration()
{
    return _config.supports_timing;
}

bool GrIPInterface::supportsCanFD()
{
    return _config.supports_canfd;
}

bool GrIPInterface::supportsTripleSampling()
{
    return false;
}

unsigned GrIPInterface::getBitrate()
{
    return _settings.bitrate();
}

uint32_t GrIPInterface::getCapabilities()
{
    uint32_t retval = 0;

    if(_manufacturer == GrIPInterface::CANIL)
    {
        retval =
            CanInterface::capability_auto_restart |
            CanInterface::capability_listen_only;
            // CanInterface::capability_config_os |
            // CanInterface::capability_auto_restart |
            //CanInterface::capability_listen_only |
            //CanInterface::capability_custom_bitrate |
            //CanInterface::capability_custom_canfd_bitrate;
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

bool GrIPInterface::updateStatistics()
{
    return updateStatus();
}

uint32_t GrIPInterface::getState()
{
    return _status.can_state;
}

int GrIPInterface::getNumRxFrames()
{
    return _status.rx_count;
}

int GrIPInterface::getNumRxErrors()
{
    return _status.rx_errors;
}

int GrIPInterface::getNumTxFrames()
{
    return _status.tx_count;
}

int GrIPInterface::getNumTxErrors()
{
    return _status.tx_errors;
}

int GrIPInterface::getNumRxOverruns()
{
    return _status.rx_overruns;
}

int GrIPInterface::getNumTxDropped()
{
    return _status.tx_dropped;
}

int GrIPInterface::getIfIndex()
{
    return _idx;
}

QString GrIPInterface::getVersion()
{
    return _version;
}

void GrIPInterface::open()
{
    if(m_GrIPHandler == nullptr)
    {
        _isOpen = false;
        _isOffline = true;
        return;
    }

    // Get Version
    for(int i = 0; i < 15; i++)
    {
        _version = QString::fromStdString(m_GrIPHandler->GetVersion());
        if(_version.size() == 0)
        {
            QThread::msleep(2);
        }
        else
        {
            break;
        }
    }

    // Close CAN port
    m_GrIPHandler->EnableChannel(_idx, false);
    QThread::msleep(2);

    if(_settings.isCustomBitrate())
    {
        QString _custombitrate = QString("%1").arg(_settings.customBitrate(), 6, 16,QLatin1Char('0')).toUpper();
        m_GrIPHandler->CAN_SetBaudrate(_idx, _custombitrate.toInt());
    }
    else
    {
        // Set the classic CAN bitrate
        switch(_settings.bitrate())
        {
            case 1000000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 1000000);
                break;
            case 800000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 800000);
                break;
            case 500000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 500000);
                break;
            case 250000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 250000);
                break;
            case 125000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 125000);
                break;
            case 100000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 100000);
                break;
            case 50000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 50000);
                break;
            case 20000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 20000);
                break;
            case 10000:
                m_GrIPHandler->CAN_SetBaudrate(_idx, 10000);
                break;
            default:
                // Default to 10k
                m_GrIPHandler->CAN_SetBaudrate(_idx, 10000);
                break;
        }
    }

    //_serport->waitForBytesWritten(20);

    // Set configured BRS rate
    /*if(_config.supports_canfd)
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
    _serport->waitForBytesWritten(20);*/

    // Set Listen Only Mode
    if(_settings.isListenOnlyMode())
    {
        m_GrIPHandler->Mode(_idx, true);
    }
    else
    {
        m_GrIPHandler->Mode(_idx, false);
    }
    /*_serport->waitForBytesWritten(100);*/

    m_GrIPHandler->SetStatus(true);

    m_GrIPHandler->SetEchoTx(true);

    m_GrIPHandler->EnableChannel(_idx, true);
    m_TxFrames.clear();

    _isOpen = true;
    _isOffline = false;
    _status.can_state = state_ok;
    _status.rx_count = 0;
    _status.rx_errors = 0;
    _status.rx_overruns = 0;
    _status.tx_count = 0;
    _status.tx_errors = 0;
    _status.tx_dropped = 0;
}

void GrIPInterface::handleSerialError(QSerialPort::SerialPortError error)
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

void GrIPInterface::close()
{
    _isOpen = false;
    _status.can_state = state_bus_off;

    m_GrIPHandler->EnableChannel(_idx, false);

    m_GrIPHandler->SetStatus(false);

    m_TxFrames.clear();
}

bool GrIPInterface::isOpen()
{
    return _isOpen;
}

void GrIPInterface::sendMessage(const CanMessage &msg)
{
    _serport_mutex.lock();

    if(m_GrIPHandler->CanTransmit(_idx, msg))
    {
        //_status.tx_count++;
        //_status.can_state = state_tx_success;

        if(msg.isShow())
        {
            //m_TxFrames.append(msg);
        }
    }
    else
    {
        _status.tx_errors++;
        _status.can_state = state_tx_fail;
    }

    _serport_mutex.unlock();
}

bool GrIPInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms)
{
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

    // Add TX frames to trace window
    /*if(m_TxFrames.size())
    {
        msglist.append(m_TxFrames);
        m_TxFrames.clear();
    }*/

    // Read all RX frames
    while(m_GrIPHandler->CanAvailable(_idx))
    {
        auto msg = m_GrIPHandler->ReceiveCan(_idx);
        if(msg.getId() != 0)
        {
            // Defaults
            msg.setInterfaceId(getId());

            if(msg.isRX() == false)
            {
                if(msg.isErrorFrame() == false)
                {
                    _status.tx_count++;
                    _status.can_state = state_tx_success;
                }
                else
                {
                    _status.tx_errors++;
                    _status.can_state = state_tx_fail;
                }

                if(msg.isShow())
                {
                    msglist.append(msg);
                }
            }
            else
            {
                msglist.append(msg);
                _status.rx_count++;
            }
        }
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
        }
    }

    // RX doesn't work on windows unless we call this for some reason
    /*_rxbuf_mutex.lock();
    if(_serport->waitForReadyRead(0))
    {
        qApp->processEvents();
    }
    _rxbuf_mutex.unlock();*/

    return true;
}
