#include "PeakCanInterface.h"
#include "PeakCanDriver.h"

#include <core/MeasurementInterface.h>
#include <windows.h>
#include "pcan-basic-api/Include/PCANBasic.h"

#include <QThread>

PeakCanInterface::PeakCanInterface(PeakCanDriver *driver, uint32_t handle)
  : CanInterface(driver),
    _handle(handle),
    _hostOffsetFirstFrame(0),
    _peakOffsetFirstFrame(0)
{
    _autoResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    _config.autoRestart = true;
    _config.listenOnly = false;
    _config.bitrate = 500000;
    _config.samplePoint = 875;

    // sample point: 50.0%
    _timings << CanTiming(0x3176,   10000, 0, 500) \
             << CanTiming(0x1876,   20000, 0, 500) \
             << CanTiming(0x0976,   50000, 0, 500) \
             << CanTiming(0x0576,   83333, 0, 500) \
             << CanTiming(0x0476,  100000, 0, 500) \
             << CanTiming(0x0376,  125000, 0, 500) \
             << CanTiming(0x0176,  250000, 0, 500) \
             << CanTiming(0x0076,  500000, 0, 500) \
             << CanTiming(0x0043,  800000, 0, 500) \
             << CanTiming(0x0032, 1000000, 0, 500);

    // sample point: 62.5%
    _timings << CanTiming(0x3158,   10000, 0, 625) \
             << CanTiming(0x1858,   20000, 0, 625) \
             << CanTiming(0x0958,   50000, 0, 625) \
             << CanTiming(0x0558,   83333, 0, 625) \
             << CanTiming(0x0458,  100000, 0, 625) \
             << CanTiming(0x0358,  125000, 0, 625) \
             << CanTiming(0x0158,  250000, 0, 625) \
             << CanTiming(0x0058,  500000, 0, 625) \
             << CanTiming(0x0034,  800000, 0, 600) \
             << CanTiming(0x0023, 1000000, 0, 625);

    // sample point: 75.0%
    _timings << CanTiming(0x313A,   10000, 0, 750) \
             << CanTiming(0x183A,   20000, 0, 750) \
             << CanTiming(0x093A,   50000, 0, 750) \
             << CanTiming(0x053A,   83333, 0, 750) \
             << CanTiming(0x043A,  100000, 0, 750) \
             << CanTiming(0x033A,  125000, 0, 750) \
             << CanTiming(0x013A,  250000, 0, 750) \
             << CanTiming(0x003A,  500000, 0, 750) \
             << CanTiming(0x0016,  800000, 0, 800) \
             << CanTiming(0x0014, 1000000, 0, 750);

    // sample point: 87.5%
    _timings << CanTiming(0x311C,   10000, 0, 875) \
             << CanTiming(0x181C,   20000, 0, 875) \
             << CanTiming(0x091C,   50000, 0, 875) \
             << CanTiming(0x051C,   83333, 0, 875) \
             << CanTiming(0x041C,  100000, 0, 875) \
             << CanTiming(0x031C,  125000, 0, 875) \
             << CanTiming(0x011C,  250000, 0, 875) \
             << CanTiming(0x001C,  500000, 0, 875) \
             << CanTiming(0x0007,  800000, 0, 900) \
             << CanTiming(0x0005, 1000000, 0, 875);

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
    _config.autoRestart = mi.doAutoRestart();
    _config.listenOnly = mi.isListenOnlyMode();
    _config.bitrate = mi.bitrate();
    _config.samplePoint = mi.samplePoint();
}

uint32_t PeakCanInterface::getCapabilities()
{
    return CanInterface::capability_listen_only | CanInterface::capability_auto_restart;
}

unsigned PeakCanInterface::getBitrate()
{
    uint32_t speed = 0;
    if (CAN_GetValue(_handle, PCAN_BUSSPEED_NOMINAL, &speed, sizeof(speed))==PCAN_ERROR_OK) {
        return speed;
    } else {
        return 0;
    }
}

uint16_t PeakCanInterface::calcBitrateMode(unsigned bitrate, unsigned samplePoint)
{
    foreach (CanTiming t, _timings) {
        if ( (t.getBitrate()==bitrate) && (t.getSamplePoint()==samplePoint) ) {
            return t.getId() & 0xFFFF;
        }
    }

    return 0;
}

QString PeakCanInterface::getErrorText(uint32_t status_code)
{
    char s[256];
    TPCANStatus result = CAN_GetErrorText(status_code, 9, s);
    if (result==PCAN_ERROR_OK) {
        return QString(s);
    } else {
        return "Unknown error";
    }
}

QList<CanTiming> PeakCanInterface::getAvailableBitrates()
{
    return _timings;
}

void PeakCanInterface::open()
{
    TPCANStatus result;

    uint32_t bitrate_adapt = PCAN_PARAMETER_ON;

    result = CAN_SetValue(_handle, PCAN_BITRATE_ADAPTING, &bitrate_adapt, sizeof(bitrate_adapt));
    if (result!=PCAN_ERROR_OK) {
        log_error(QString("could not set bitrate adapt parameter for CAN channel %1: %2").arg(getName()).arg(getErrorText(result)));
    }

    uint32_t listen_only = _config.listenOnly ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    result = CAN_SetValue(_handle, PCAN_LISTEN_ONLY, &listen_only, sizeof(listen_only));
    if (result!=PCAN_ERROR_OK) {
        log_error(QString("could not set listen only mode=%3 for CAN channel %1: %2").arg(getName()).arg(getErrorText(result)).arg(_config.listenOnly ? "on" : "off"));
    }

    uint16_t bitrate_mode = calcBitrateMode(_config.bitrate, _config.samplePoint);
    if (bitrate_mode==0) {
        log_error(
            QString("CAN channel %1: cannot find bitrate settings for baud rate %2, sample point %3")
            .arg(getName())
            .arg(_config.bitrate)
            .arg(CanTiming::getSamplePointStr(_config.samplePoint))
        );
        return;
    }

    result = CAN_Initialize(_handle, bitrate_mode);

    if ( (result!=PCAN_ERROR_OK) && (result != PCAN_ERROR_CAUTION) ) {
        log_error(QString("could not initialize CAN channel %1: %2").arg(getName()).arg(getErrorText(result)));
        return;
    }

    log_info(QString("CAN channel %1 initialized").arg(getName()));
    if (getBitrate() == _config.bitrate) {
        log_info(QString("setting bitrate of CAN channel %1 to %2").arg(getName()).arg(getBitrate()));
    } else {
        log_warning(QString("could not set correct bitrate for CAN channel %1: bitrate is now %2 instead of configured %3").arg(getName()).arg(getBitrate()).arg(_config.bitrate));
    }

    if (CAN_SetValue(_handle, PCAN_RECEIVE_EVENT, &_autoResetEvent, sizeof(_autoResetEvent))!=PCAN_ERROR_OK) {
        log_error(QString("could not set read event for CAN channel: %1").arg(getName()));
    }

    uint32_t autoreset = _config.autoRestart ? PCAN_PARAMETER_ON : PCAN_PARAMETER_OFF;
    result = CAN_SetValue(_handle, PCAN_BUSOFF_AUTORESET, &autoreset, sizeof(autoreset));
    if (result!=PCAN_ERROR_OK) {
        log_error(QString("could not set busoff auto reset mode=%3 for CAN channel %1: %2").arg(getName()).arg(getErrorText(result)).arg(_config.autoRestart ? "on" : "off"));
    }

    _peakOffsetFirstFrame = 0;
    _hostOffsetFirstFrame = 0;

}

void PeakCanInterface::close()
{
    CAN_Uninitialize(_handle);
    log_info(QString("CAN channel %1 uninitialized").arg(getName()));
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

        // Total Microseconds = micros + 1000 * millis + 0x100000000 * 1000 * millis_overflow
        uint64_t ts = timestamp.millis;
        ts += 0x100000000 * (uint64_t)timestamp.millis_overflow;
        ts *= 1000;
        ts += timestamp.micros;

        if ( (_peakOffsetFirstFrame==0) || (_hostOffsetFirstFrame==0) ) {
            _hostOffsetFirstFrame = getDriver()->backend().getUsecsAtMeasurementStart()
                                  + getDriver()->backend().getUsecsSinceMeasurementStart();
            _peakOffsetFirstFrame = ts;
        }

        ts -=_peakOffsetFirstFrame;
        ts += _hostOffsetFirstFrame;

        msg.setTimestamp(ts/1000000, ts % 1000000);

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
