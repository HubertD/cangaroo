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

#ifndef CANDLEAPIDEVICE_H
#define CANDLEAPIDEVICE_H

#include "api/candle.h"
#include <QMutex>
#include <QMap>
#include <QQueue>
#include <string>

/**
 * Shared owner of a single candle (gs_usb) USB device handle.
 *
 * A gs_usb device can expose several CAN channels through one USB handle, but
 * each CANgaroo CandleApiInterface runs in its own CanListener thread. Routing
 * every channel's I/O through this object keeps the single handle thread-safe:
 * the device is opened once (ref-counted), reads are serialized and dispatched
 * to the owning channel by the frame's channel field, and sends are serialized.
 *
 * ponytail: one coarse device-wide mutex guards open/close/read/send. A
 * dedicated per-device reader thread would lower latency if it ever matters.
 */
class CandleApiDevice
{
public:
    explicit CandleApiDevice(candle_handle handle);
    ~CandleApiDevice();

    candle_handle handle() const { return _handle; }
    std::wstring path() const;
    uint8_t channelCount() const { return _channelCount; }

    // Ref-counted device open. First caller opens the USB device and latches the
    // timestamp base; the last releaser closes it. Returns false on failure.
    bool acquire(uint64_t hostNowUs);
    void release();

    bool startChannel(uint8_t ch, uint32_t flags);
    void stopChannel(uint8_t ch);

    bool send(uint8_t ch, candle_frame_t *frame);

    // Returns one received frame destined for channel ch. Frames for other
    // channels read off the wire are buffered for their owner. Returns false if
    // nothing for ch arrived within the (internally capped) timeout.
    bool read(uint8_t ch, candle_frame_t *out, uint32_t timeout_ms);

    uint64_t hostOffsetStart() const { return _hostOffsetStart; }
    uint32_t deviceTicksStart() const { return _deviceTicksStart; }

private:
    // Briefly open/close the device so channel count and capabilities
    // (fclk_can, feature flags) are read from hardware and cached. candle_dev_*
    // leave these populated after close, so the setup dialog can query bitrates
    // and capabilities while the interface is otherwise closed.
    void probeInfo();

    candle_handle _handle;
    uint8_t _channelCount;
    int _openCount;
    bool _timestampInit;
    uint64_t _hostOffsetStart;
    uint32_t _deviceTicksStart;
    QMap<uint8_t, QQueue<candle_frame_t>> _rxq;
    QMutex _mutex;
};

#endif // CANDLEAPIDEVICE_H
