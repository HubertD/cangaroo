#include "PeakCanInterface.h"
#include "PeakCanDriver.h"

#include <windows.h>
#include "pcan-basic-api/Include/PCANBasic.h"

#include <QThread>

PeakCanInterface::PeakCanInterface(PeakCanDriver *driver, uint32_t handle)
  : CanInterface(driver),
    _handle(handle)
{
    _autoResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

PeakCanInterface::~PeakCanInterface()
{
    CloseHandle(_autoResetEvent);
}

void PeakCanInterface::update()
{
    // TODO: implement me
}

uint32_t PeakCanInterface::getHandle() const
{
    return _handle;
}

QString PeakCanInterface::getName() const
{
    switch (_handle) {
        case PCAN_USBBUS1:
        case PCAN_USBBUS2:
        case PCAN_USBBUS3:
        case PCAN_USBBUS4:
        case PCAN_USBBUS5:
        case PCAN_USBBUS6:
        case PCAN_USBBUS7:
        case PCAN_USBBUS8:
            return QString("pusb%1").arg(_handle - PCAN_USBBUS1 + 1);
        default:
            return QString("pcan_%1").arg(_handle);
    }

    /*
    char name[32];
    if (CAN_GetValue(_handle, PCAN_HARDWARE_NAME, name, sizeof(name))==PCAN_ERROR_OK) {
        return QString(name);
    } else {
        return "";
    }
*/
}

void PeakCanInterface::applyConfig(const MeasurementInterface &mi)
{
    // TODO: implement me
}

int PeakCanInterface::getBitrate()
{
    uint32_t speed = 0;
    if (CAN_GetValue(_handle, PCAN_BUSSPEED_NOMINAL, &speed, sizeof(speed))==PCAN_ERROR_OK) {
        return speed;
    } else {
        return 0;
    }
}

QList<int> PeakCanInterface::getAvailableBitrates()
{
    QList<int> retval;
    retval << 5000 << 10000 << 20000 << 33333 << 47619 << 50000 << 83333 << 95238 << 100000 << 125000 << 250000 << 500000 << 800000 << 1000000;
    return retval;
}

QList<int> PeakCanInterface::getAvailableFdBitrates()
{
    return CanInterface::getAvailableFdBitrates();
}

void PeakCanInterface::open()
{
    if (CAN_Initialize(_handle, PCAN_BAUD_500K)==PCAN_ERROR_OK) {
        Backend::instance().logMessage(log_level_info, QString("CAN channel %1 initialized").arg(getName()));
    } else {
        Backend::instance().logMessage(log_level_error, QString("could not initialize CAN channel: %1").arg(getName()));
    }

    if (CAN_SetValue(_handle, PCAN_RECEIVE_EVENT, &_autoResetEvent, sizeof(_autoResetEvent))!=PCAN_ERROR_OK) {
        Backend::instance().logMessage(log_level_error, QString("could not set read event for CAN channel: %1").arg(getName()));
    }
}

void PeakCanInterface::close()
{
    CAN_Uninitialize(_handle);
    Backend::instance().logMessage(log_level_info, QString("CAN channel %1 uninitialized").arg(getName()));
}

uint32_t PeakCanInterface::getState()
{
    // TODO implement me
    return state_unknown;
}

bool PeakCanInterface::readMessage(CanMessage &msg, unsigned int timeout_ms)
{
    TPCANMsg buf;
    TPCANTimestamp timestamp;

    if (WaitForSingleObject(_autoResetEvent, timeout_ms) != WAIT_OBJECT_0) {
        return false;
    }

    TPCANStatus result = CAN_Read(_handle, &buf, &timestamp);
    if (result == PCAN_ERROR_OK) {
        if ((buf.MSGTYPE & PCAN_MESSAGE_STATUS) != 0) {
            // TODO handle status message?
            return false;
        } else {
            msg.setErrorFrame(false);
        }

        msg.setInterfaceId(getId());
        msg.setId(buf.ID);
        msg.setExtended((buf.MSGTYPE & PCAN_MESSAGE_EXTENDED)!=0);
        msg.setRTR((buf.MSGTYPE & PCAN_MESSAGE_RTR)!=0);

        uint8_t len = (buf.LEN > 8) ? 8 : buf.LEN;
        msg.setLength(len);
        for (int i=0; i<len; i++) {
            msg.setByte(i, buf.DATA[i]);
        }

        // FIXME set correct timestamp
        uint64_t ms = (uint64_t)timestamp.millis + 0x100000000 * (uint64_t)timestamp.millis_overflow;
        msg.setTimestamp(ms/1000, (1000*(ms%1000)) + timestamp.micros);

        return true;
    } else {
        QThread::msleep(10);
        return false;
    }
}

void PeakCanInterface::sendMessage(const CanMessage &msg)
{
    TPCANMsg buf;
    buf.ID = msg.getId();
    buf.MSGTYPE = 0;
    if (msg.isExtended()) {
        buf.MSGTYPE |= PCAN_MESSAGE_EXTENDED;
    }
    if (msg.isRTR()) {
        buf.MSGTYPE |= PCAN_MESSAGE_EXTENDED;
    }

    buf.LEN = msg.getLength();
    for (int i=0; i<8; i++) {
        buf.DATA[i] = msg.getByte(i);
    }
    CAN_Write(_handle, &buf);
    // TODO check error, update statistics (increment txerrors or txframes)
}

bool PeakCanInterface::updateStatistics()
{
    return true;
}

int PeakCanInterface::getNumTxDropped()
{
    return 0;
}

int PeakCanInterface::getNumRxOverruns()
{
    return 0;
}

int PeakCanInterface::getNumTxErrors()
{
    return 0;
}

int PeakCanInterface::getNumTxFrames()
{
    return 0;
}

int PeakCanInterface::getNumRxErrors()
{
    return 0;
}

int PeakCanInterface::getNumRxFrames()
{
    return 0;
}
