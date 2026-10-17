/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

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

#include "CandleApiDevice.h"
#include <QtGlobal>

CandleApiDevice::CandleApiDevice(candle_handle handle)
  : _handle(handle),
    _channelCount(1),
    _openCount(0),
    _timestampInit(false),
    _hostOffsetStart(0),
    _deviceTicksStart(0)
{
    probeInfo();
}

CandleApiDevice::~CandleApiDevice()
{
    candle_dev_free(_handle);
}

std::wstring CandleApiDevice::path() const
{
    return std::wstring(candle_dev_get_path(_handle));
}

void CandleApiDevice::probeInfo()
{
    if (candle_dev_open(_handle)) {
        uint8_t n = 1;
        if (candle_channel_count(_handle, &n) && n >= 1) {
            _channelCount = n;
        }
        candle_dev_close(_handle);
    }
}

bool CandleApiDevice::acquire(uint64_t hostNowUs)
{
    QMutexLocker lock(&_mutex);

    if (_openCount == 0) {
        if (!candle_dev_open(_handle)) {
            return false;
        }
        _timestampInit = false;
        uint32_t t_dev;
        if (candle_dev_get_timestamp_us(_handle, &t_dev)) {
            _hostOffsetStart = hostNowUs;
            _deviceTicksStart = t_dev;
            _timestampInit = true;
        }
    }

    _openCount++;
    return true;
}

void CandleApiDevice::release()
{
    QMutexLocker lock(&_mutex);

    if (_openCount <= 0) {
        return;
    }

    if (--_openCount == 0) {
        candle_dev_close(_handle);
        _rxq.clear();
    }
}

bool CandleApiDevice::startChannel(uint8_t ch, uint32_t flags)
{
    QMutexLocker lock(&_mutex);
    return candle_channel_start(_handle, ch, flags);
}

void CandleApiDevice::stopChannel(uint8_t ch)
{
    QMutexLocker lock(&_mutex);
    candle_channel_stop(_handle, ch);
    _rxq.remove(ch);
}

bool CandleApiDevice::send(uint8_t ch, candle_frame_t *frame)
{
    QMutexLocker lock(&_mutex);
    return candle_frame_send(_handle, ch, frame);
}

bool CandleApiDevice::read(uint8_t ch, candle_frame_t *out, uint32_t timeout_ms)
{
    QMutexLocker lock(&_mutex);

    // Deliver anything already buffered for this channel first.
    QQueue<candle_frame_t> &own = _rxq[ch];
    if (!own.isEmpty()) {
        *out = own.dequeue();
        return true;
    }

    // Cap how long we hold the device lock during a blocking read so sibling
    // channels stay responsive; the CanListener loop re-polls.
    uint32_t t = qMin(timeout_ms, 100u);

    candle_frame_t f;
    if (candle_frame_read(_handle, &f, t)) {
        if (f.channel == ch) {
            *out = f;
            return true;
        }
        // Frame for a different channel: buffer it for its owner.
        if (f.channel < _channelCount) {
            _rxq[f.channel].enqueue(f);
        }
    }

    return false;
}
