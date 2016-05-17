#include "PeakCanInterface.h"
#include "PeakCanDriver.h"

#include <core/MeasurementInterface.h>
#include <windows.h>
#include "pcan-basic-api/Include/PCANBasic.h"

#include <QThread>

PeakCanInterface::PeakCanInterface(PeakCanDriver *driver, uint32_t handle)
  : CanInterface(driver),
    _handle(handle),
    _hostTimestampStart(0),
    _peakTimestampStart(0)
{

    LARGE_INTEGER tps;
    QueryPerformanceFrequency(&tps);
    _perfTicksPerSecond = tps.QuadPart;

    _autoResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    _config.autoRestart = true;
    _config.listenOnly = false;
    _config.bitrate = 100000;
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

}

uint32_t PeakCanInterface::getCapabilities()
{
    return CanInterface::capability_listen_only | CanInterface::capability_auto_restart;
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

uint16_t PeakCanInterface::calcBitrateMode(int bitrate)
{
    switch (bitrate) {
        case    5000: return PCAN_BAUD_5K;
        case   10000: return PCAN_BAUD_10K;
        case   20000: return PCAN_BAUD_20K;
        case   33333: return PCAN_BAUD_33K;
        case   47619: return PCAN_BAUD_47K;
        case   50000: return PCAN_BAUD_50K;
        case   83333: return PCAN_BAUD_83K;
        case   95238: return PCAN_BAUD_95K;
        case  100000: return PCAN_BAUD_100K;
        case  125000: return PCAN_BAUD_125K;
        case  250000: return PCAN_BAUD_250K;
        case  500000: return PCAN_BAUD_500K;
        case  800000: return PCAN_BAUD_800K;
        case 1000000: return PCAN_BAUD_1M;
        default: return 0;
    }
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
    return CanInterface::getAvailableBitrates();
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

    uint16_t bitrate_mode = calcBitrateMode(_config.bitrate);
    if (bitrate_mode==0) {
        log_error(QString("CAN channel %1: cannot find bitrate settings for baud rate: %2").arg(getName()).arg(bitrate_mode));
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

        if (_peakTimestampStart==0) {
            _hostTimestampStart = 1000 * QDateTime::currentMSecsSinceEpoch();
            _peakTimestampStart = ts;
        }

        ts -=_peakTimestampStart;
        ts += _hostTimestampStart;

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
