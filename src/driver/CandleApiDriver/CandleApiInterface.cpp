#include "CandleApiDriver.h"
#include "CandleApiInterface.h"
#include <windows.h>

CandleApiInterface::CandleApiInterface(CandleApiDriver *driver, candle_handle handle)
  : CanInterface(driver),
   _handle(handle),
   _bitrate(500000),
   _start_ts_us(0),
   _t_start_ms(0)
{

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

void CandleApiInterface::open()
{
    if (!candle_dev_open(_handle)) {
        // DO what?
        return;
    }

    candle_dev_get_timestamp_us(_handle, &_start_ts_us);
    _t_start_ms = QDateTime::currentMSecsSinceEpoch();

    candle_channel_set_bitrate(_handle, 0, _bitrate);
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

            int64_t ts_us = candle_frame_timestamp_us(&frame) - _start_ts_us; // TODO overflows...
            uint64_t ms = _t_start_ms + (ts_us/1000);
            msg.setTimestamp(ms/1000, ts_us % 1000000);

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
