#include "CandleApiDriver.h"
#include "CandleApiInterface.h"

CandleApiInterface::CandleApiInterface(CandleApiDriver *driver, candle_handle handle)
  : CanInterface(driver),
    _perfCountStart(0),
    _deviceTicksLastSync(0),
    _tLastSync_us(0),
    _handle(handle),
    _bitrate(500000)
{
    LARGE_INTEGER tps;
    QueryPerformanceFrequency(&tps);
    _perfTicksPerSecond = tps.QuadPart;
}

CandleApiInterface::~CandleApiInterface()
{

}

QString CandleApiInterface::getName() const
{
    return "candle";
}

void CandleApiInterface::applyConfig(const MeasurementInterface &mi)
{
}

int CandleApiInterface::getBitrate()
{
    return _bitrate;
}

QList<int> CandleApiInterface::getAvailableBitrates()
{
    QList<int> retval;
    retval << 10000 << 20000 << 50000 << 83333 << 100000 << 125000 << 250000 << 500000 << 800000 << 1000000;
    return retval;
}

QList<int> CandleApiInterface::getAvailableFdBitrates()
{
    return CanInterface::getAvailableFdBitrates();
}

//! sync device time with host time
/*!
 * fetch the current device timestamp and
 * save the corresponing host time to be able
 * to calculate exact receive timestamps.
 *
 * should be called regulary to ensure that
 * host and device time do not run out of sync
 */
void CandleApiInterface::syncTimestamp()
{
    uint32_t t_dev;
    uint64_t t;
    LARGE_INTEGER pc;

    if (candle_dev_get_timestamp_us(_handle, &t_dev)) {
        QueryPerformanceCounter(&pc);

        t = pc.QuadPart - _perfCountStart;
        t *= 1000000;
        t /= _perfTicksPerSecond;

        _tLastSync_us = t;
        _deviceTicksLastSync = t_dev;
    }
}

//! sync device and host timestamps if neccessary
void CandleApiInterface::checkSyncTimestamp()
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);

    uint64_t t = pc.QuadPart - _perfCountStart;
    t *= 1000000;
    t /= _perfTicksPerSecond;

    // resync if last sync is older than 10sec
    if ( (t - _tLastSync_us) > 10000000 ) {
        syncTimestamp();
    }

}

void CandleApiInterface::open()
{
    if (!candle_dev_open(_handle)) {
        // DO what?
        return;
    }

    LARGE_INTEGER pc;
    candle_channel_set_bitrate(_handle, 0, _bitrate);
    QueryPerformanceCounter(&pc);
    _perfCountStart = pc.QuadPart;
    syncTimestamp();
    candle_channel_start(_handle, 0, 0);
}

void CandleApiInterface::close()
{
    candle_channel_stop(_handle, 0);
    candle_dev_close(_handle);
}

void CandleApiInterface::sendMessage(const CanMessage &msg)
{
}

bool CandleApiInterface::readMessage(CanMessage &msg, unsigned int timeout_ms)
{
    candle_frame_t frame;

    if (candle_frame_read(_handle, &frame, timeout_ms)) {

        if (candle_frame_type(&frame)==CANDLE_FRAMETYPE_RECEIVE) {

            msg.setInterfaceId(getId());
            msg.setErrorFrame(false);
            msg.setId(candle_frame_id(&frame));
            msg.setExtended(candle_frame_is_extended_id(&frame));
            msg.setRTR(candle_frame_is_rtr(&frame));

            uint8_t dlc = candle_frame_dlc(&frame);
            uint8_t *data = candle_frame_data(&frame);
            msg.setLength(dlc);
            for (int i=0; i<dlc; i++) {
                msg.setByte(i, data[i]);
            }

            checkSyncTimestamp();

            uint32_t dev_ts = candle_frame_timestamp_us(&frame);
            dev_ts -= _deviceTicksLastSync;

            uint64_t ts_us = _tLastSync_us + dev_ts;
            msg.setTimestamp(ts_us/1000000, ts_us % 1000000);

            return true;
        }

    }

    return false;
}

bool CandleApiInterface::updateStatistics()
{
    return true;
}

uint32_t CandleApiInterface::getState()
{
    return CanInterface::state_ok;
}

int CandleApiInterface::getNumRxFrames()
{
    return 0;
}

int CandleApiInterface::getNumRxErrors()
{
    return 0;
}

int CandleApiInterface::getNumTxFrames()
{
    return 0;
}

int CandleApiInterface::getNumTxErrors()
{
    return 0;
}

int CandleApiInterface::getNumRxOverruns()
{
    return 0;
}

int CandleApiInterface::getNumTxDropped()
{
    return 0;
}

wstring CandleApiInterface::getPath()
{
    return wstring(candle_dev_get_path(_handle));
}

void CandleApiInterface::update(candle_handle dev)
{
    candle_dev_free(_handle);
    _handle = dev;
}

