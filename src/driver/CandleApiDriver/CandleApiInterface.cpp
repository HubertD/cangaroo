#include "CandleApiDriver.h"
#include "CandleApiInterface.h"

CandleApiInterface::CandleApiInterface(CandleApiDriver *driver, CandleApiDevice *device, uint8_t channel)
  : CanInterface(driver),
    _isOpen(false),
    _device(device),
    _ch(channel),
    _settings(),
    _backend(driver->backend()),
    _numRx(0),
    _numTx(0),
    _numTxErr(0)
{
    _settings.setBitrate(500000);
    _settings.setSamplePoint(875);



    // Timings for 170MHz processors (CANable 2.0)
    // Tseg1: 2..256 Tseg2: 2..128 sjw: 1..128 brp: 1..512
    // Note: as expressed below, Tseg1 does not include 1 count for prop phase
    _timings
        << CandleApiTiming(170000000,   10000, 875, 68, 217, 31)
        << CandleApiTiming(170000000,   20000, 875, 34, 217, 31)
        << CandleApiTiming(170000000,   50000, 875, 17, 173, 25)
        << CandleApiTiming(170000000,   83333, 875,  8, 221, 32)
        << CandleApiTiming(170000000,  100000, 875, 10, 147, 21)
        << CandleApiTiming(170000000,  125000, 875, 8,  147, 21)
        << CandleApiTiming(170000000,  250000, 875, 4,  147, 21)
        << CandleApiTiming(170000000,  500000, 875, 2,  147, 21)
        << CandleApiTiming(170000000, 1000000, 875, 1,  147, 21);


    // Timings for 48MHz processors (CANable 0.X)
    _timings
        // sample point: 50.0%
        << CandleApiTiming(48000000,   10000, 500, 300, 6, 8)
        << CandleApiTiming(48000000,   20000, 500, 150, 6, 8)
        << CandleApiTiming(48000000,   50000, 500,  60, 6, 8)
        << CandleApiTiming(48000000,   83333, 500,  36, 6, 8)
        << CandleApiTiming(48000000,  100000, 500,  30, 6, 8)
        << CandleApiTiming(48000000,  125000, 500,  24, 6, 8)
        << CandleApiTiming(48000000,  250000, 500,  12, 6, 8)
        << CandleApiTiming(48000000,  500000, 500,   6, 6, 8)
        << CandleApiTiming(48000000,  800000, 500,   3, 8, 9)
        << CandleApiTiming(48000000, 1000000, 500,   3, 6, 8)

        // sample point: 62.5%
        << CandleApiTiming(48000000,   10000, 625, 300, 8, 6)
        << CandleApiTiming(48000000,   20000, 625, 150, 8, 6)
        << CandleApiTiming(48000000,   50000, 625,  60, 8, 6)
        << CandleApiTiming(48000000,   83333, 625,  36, 8, 6)
        << CandleApiTiming(48000000,  100000, 625,  30, 8, 6)
        << CandleApiTiming(48000000,  125000, 625,  24, 8, 6)
        << CandleApiTiming(48000000,  250000, 625,  12, 8, 6)
        << CandleApiTiming(48000000,  500000, 625,   6, 8, 6)
        << CandleApiTiming(48000000,  800000, 600,   4, 7, 6)
        << CandleApiTiming(48000000, 1000000, 625,   3, 8, 6)

        // sample point: 75.0%
        << CandleApiTiming(48000000,   10000, 750, 300, 10, 4)
        << CandleApiTiming(48000000,   20000, 750, 150, 10, 4)
        << CandleApiTiming(48000000,   50000, 750,  60, 10, 4)
        << CandleApiTiming(48000000,   83333, 750,  36, 10, 4)
        << CandleApiTiming(48000000,  100000, 750,  30, 10, 4)
        << CandleApiTiming(48000000,  125000, 750,  24, 10, 4)
        << CandleApiTiming(48000000,  250000, 750,  12, 10, 4)
        << CandleApiTiming(48000000,  500000, 750,   6, 10, 4)
        << CandleApiTiming(48000000,  800000, 750,   3, 13, 5)
        << CandleApiTiming(48000000, 1000000, 750,   3, 10, 4)

        // sample point: 87.5%
        << CandleApiTiming(48000000,   10000, 875, 300, 12, 2)
        << CandleApiTiming(48000000,   20000, 875, 150, 12, 2)
        << CandleApiTiming(48000000,   50000, 875,  60, 12, 2)
        << CandleApiTiming(48000000,   83333, 875,  36, 12, 2)
        << CandleApiTiming(48000000,  100000, 875,  30, 12, 2)
        << CandleApiTiming(48000000,  125000, 875,  24, 12, 2)
        << CandleApiTiming(48000000,  250000, 875,  12, 12, 2)
        << CandleApiTiming(48000000,  500000, 875,   6, 12, 2)
        << CandleApiTiming(48000000,  800000, 867,   4, 11, 2)
        << CandleApiTiming(48000000, 1000000, 875,   3, 12, 2);


    _timings
        // sample point: 50.0%
        << CandleApiTiming(16000000,   10000, 520, 64, 11, 12)
        << CandleApiTiming(16000000,   20000, 500, 50,  6,  8)
        << CandleApiTiming(16000000,   50000, 500, 20,  6,  8)
        << CandleApiTiming(16000000,   83333, 500, 12,  6,  8)
        << CandleApiTiming(16000000,  100000, 500, 10,  6,  8)
        << CandleApiTiming(16000000,  125000, 500,  8,  6,  8)
        << CandleApiTiming(16000000,  250000, 500,  4,  6,  8)
        << CandleApiTiming(16000000,  500000, 500,  2,  6,  8)
        << CandleApiTiming(16000000,  800000, 500,  1,  8, 10)
        << CandleApiTiming(16000000, 1000000, 500,  1,  6,  8)

        // sample point: 62.5%
        << CandleApiTiming(16000000,   10000, 625, 64, 14,  9)
        << CandleApiTiming(16000000,   20000, 625, 50,  8,  6)
        << CandleApiTiming(16000000,   50000, 625, 20,  8,  6)
        << CandleApiTiming(16000000,   83333, 625, 12,  8,  6)
        << CandleApiTiming(16000000,  100000, 625, 10,  8,  6)
        << CandleApiTiming(16000000,  125000, 625,  8,  8,  6)
        << CandleApiTiming(16000000,  250000, 625,  4,  8,  6)
        << CandleApiTiming(16000000,  500000, 625,  2,  8,  6)
        << CandleApiTiming(16000000,  800000, 625,  1, 11,  7)
        << CandleApiTiming(16000000, 1000000, 625,  1,  8,  6)

        // sample point: 75.0%
        << CandleApiTiming(16000000,   20000, 750, 50, 10,  4)
        << CandleApiTiming(16000000,   50000, 750, 20, 10,  4)
        << CandleApiTiming(16000000,   83333, 750, 12, 10,  4)
        << CandleApiTiming(16000000,  100000, 750, 10, 10,  4)
        << CandleApiTiming(16000000,  125000, 750,  8, 10,  4)
        << CandleApiTiming(16000000,  250000, 750,  4, 10,  4)
        << CandleApiTiming(16000000,  500000, 750,  2, 10,  4)
        << CandleApiTiming(16000000,  800000, 750,  1, 13,  5)
        << CandleApiTiming(16000000, 1000000, 750,  1, 10,  4)

        // sample point: 87.5%
        << CandleApiTiming(16000000,   20000, 875, 50, 12,  2)
        << CandleApiTiming(16000000,   50000, 875, 20, 12,  2)
        << CandleApiTiming(16000000,   83333, 875, 12, 12,  2)
        << CandleApiTiming(16000000,  100000, 875, 10, 12,  2)
        << CandleApiTiming(16000000,  125000, 875,  8, 12,  2)
        << CandleApiTiming(16000000,  250000, 875,  4, 12,  2)
        << CandleApiTiming(16000000,  500000, 875,  2, 12,  2)
        << CandleApiTiming(16000000,  800000, 900,  2,  7,  1)
        << CandleApiTiming(16000000, 1000000, 875,  1, 12,  2);

    _timings
        // Timings for 80MHz (CANnectivity)
        << CandleApiTiming(80000000,   10000, 875, 80,  86, 13)
        << CandleApiTiming(80000000,   20000, 875, 40,  86, 13)
        << CandleApiTiming(80000000,   50000, 875, 16,  86, 13)
        << CandleApiTiming(80000000,  100000, 875,  8,  86, 13)
        << CandleApiTiming(80000000,  125000, 875,  6,  86, 13)
        << CandleApiTiming(80000000,  250000, 875,  3,  86, 13)
        << CandleApiTiming(80000000,  500000, 875,  1, 139, 20)
        << CandleApiTiming(80000000,  800000, 875,  1,  86, 13)
        << CandleApiTiming(80000000, 1000000, 875,  1,  69, 10)
        << CandleApiTiming(80000000, 2000000, 875,  1,  34,  5)
        << CandleApiTiming(80000000, 4000000, 850,  1,  16,  3)
        << CandleApiTiming(80000000, 5000000, 875,  1,  13,  2)
        << CandleApiTiming(80000000, 8000000, 800,  1,   7,  2)

        // Timings for 160MHz (CandleLight 2.5)
        << CandleApiTiming(160000000,   10000, 875, 160,  86, 13)
        << CandleApiTiming(160000000,   20000, 875, 80,  86, 13)
        << CandleApiTiming(160000000,   50000, 875, 32,  86, 13)
        << CandleApiTiming(160000000,  100000, 875, 16,  86, 13)
        << CandleApiTiming(160000000,  125000, 875, 12,  86, 13)
        << CandleApiTiming(160000000,  250000, 875,  6,  86, 13)
        << CandleApiTiming(160000000,  500000, 875,  3,  86, 13)
        << CandleApiTiming(160000000,  800000, 875,  1, 174, 25)
        << CandleApiTiming(160000000, 1000000, 875,  1, 139, 20)
        << CandleApiTiming(160000000, 2000000, 875,  1,  69, 10)
        << CandleApiTiming(160000000, 4000000, 875,  1,  34,  5)
        << CandleApiTiming(160000000, 5000000, 875,  1,  27,  4)
        << CandleApiTiming(160000000, 8000000, 850,  1,  16,  3);
}

CandleApiInterface::~CandleApiInterface()
{

}

QString CandleApiInterface::getName() const
{
    return "candle" + QString::number(getId() & 0xFF) + ".ch" + QString::number(_ch);
}

QString CandleApiInterface::getDetailsStr() const
{
    return QString::fromStdWString(getPath()) + " (channel " + QString::number(_ch) + ")";
}

void CandleApiInterface::applyConfig(const MeasurementInterface &mi)
{
    _settings = mi;
}

unsigned CandleApiInterface::getBitrate()
{
    return _settings.bitrate();
}

uint32_t CandleApiInterface::getCapabilities()
{
    candle_capability_t caps;

    if (candle_channel_get_capabilities(_device->handle(), _ch, &caps)) {

        uint32_t retval = 0;

        if (caps.feature & CANDLE_MODE_LISTEN_ONLY) {
            retval |= CanInterface::capability_listen_only;
        }

        if (caps.feature & CANDLE_MODE_ONE_SHOT) {
            retval |= CanInterface::capability_one_shot;
        }

        if (caps.feature & CANDLE_MODE_TRIPLE_SAMPLE) {
            retval |= CanInterface::capability_triple_sampling;
        }

        if (caps.feature & CANDLE_FEATURE_FD) {
            retval |= CanInterface::capability_canfd;
        }

        return retval;

    } else {
        return 0;
    }
}

QList<CanTiming> CandleApiInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;

    candle_capability_t caps;
    if (candle_channel_get_capabilities(_device->handle(), _ch, &caps)) {
        int i = 0;
        foreach (const CandleApiTiming t, _timings) {
            if (t.getBaseClk() == caps.fclk_can) {
                retval << CanTiming(i++, t.getBitrate(), 0, t.getSamplePoint());
            }
        }
    }

    return retval;
}

bool CandleApiInterface::setBitTiming(uint32_t bitrate, uint32_t samplePoint, uint32_t fdBitrate, uint32_t fdSamplePoint, bool useFD)
{
    candle_capability_t caps;
    if (!candle_channel_get_capabilities(_device->handle(), _ch, &caps)) {
        return false;
    }

    bool timing_found = false;
    foreach (const CandleApiTiming t, _timings) {
        if ( (t.getBaseClk() == caps.fclk_can)
          && (t.getBitrate()==bitrate)
          && (t.getSamplePoint()==samplePoint) )
        {
            candle_bittiming_t timing = t.getTiming();
            if (!candle_channel_set_timing(_device->handle(), _ch, &timing)) {
                return false;
            }
            timing_found = true;
            break;
        }
    }

    if (!timing_found) {
        return false;
    }

    if (useFD && (caps.feature & CANDLE_FEATURE_FD)) {
        bool data_timing_found = false;
        foreach (const CandleApiTiming t, _timings) {
            if ( (t.getBaseClk() == caps.fclk_can)
              && (t.getBitrate()==fdBitrate)
              && (t.getSamplePoint()==fdSamplePoint) )
            {
                candle_bittiming_t timing = t.getTiming();
                if (!candle_channel_set_data_timing(_device->handle(), _ch, &timing)) {
                    return false;
                }
                data_timing_found = true;
                break;
            }
        }
        if (!data_timing_found) {
            return false;
        }
    }

    return true;
}

void CandleApiInterface::open()
{
    uint64_t hostNow =
            _backend.getUsecsAtMeasurementStart() +
            _backend.getUsecsSinceMeasurementStart();

    if (!_device->acquire(hostNow)) {
        _isOpen = false;
        return;
    }

    if (!setBitTiming(_settings.bitrate(), _settings.samplePoint(), _settings.fdBitrate(), _settings.fdSamplePoint(), _settings.isCanFD())) {
        _device->release();
        _isOpen = false;
        return;
    }

    uint32_t flags = 0;
    if (_settings.isCanFD()) {
        flags |= CANDLE_MODE_FD;
    }
    if (_settings.isListenOnlyMode()) {
        flags |= CANDLE_MODE_LISTEN_ONLY;
    }
    if (_settings.isOneShotMode()) {
        flags |= CANDLE_MODE_ONE_SHOT;
    }
    if (_settings.isTripleSampling()) {
        flags |= CANDLE_MODE_TRIPLE_SAMPLE;
    }

    _numRx = 0;
    _numTx = 0;
    _numTxErr = 0;

    if (!_device->startChannel(_ch, flags)) {
        _device->release();
        _isOpen = false;
        return;
    }
    _isOpen = true;
}

bool CandleApiInterface::isOpen()
{
    return _isOpen;
}

void CandleApiInterface::close()
{
    _device->stopChannel(_ch);
    _device->release();
    _isOpen = false;
}

void CandleApiInterface::sendMessage(const CanMessage &msg)
{
    candle_frame_t frame;

    frame.can_id = msg.getId();
    if (msg.isExtended()) {
        frame.can_id |= CANDLE_ID_EXTENDED;
    }
    if (msg.isRTR()) {
        frame.can_id |= CANDLE_ID_RTR;
    }

    if (msg.isFD()) {
        frame.flags |= CANDLE_FLAG_FD;
    }
    if (msg.isBRS()) {
        frame.flags |= CANDLE_FLAG_BRS;
    }

    frame.can_dlc = msg.getLength();
    for (int i=0; i<frame.can_dlc && i<64; i++) {
        frame.data[i] = msg.getByte(i);
    }

    if (_device->send(_ch, &frame)) {
        _numTx++;
    } else {
        _numTxErr++;
    }
}

bool CandleApiInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms)
{
    candle_frame_t frame;
    CanMessage msg;

    if (_device->read(_ch, &frame, timeout_ms)) {

        if (candle_frame_type(&frame)==CANDLE_FRAMETYPE_RECEIVE) {
            _numRx++;

            msg.setInterfaceId(getId());
            msg.setErrorFrame(false);
            msg.setId(candle_frame_id(&frame));
            msg.setExtended(candle_frame_is_extended_id(&frame));
            msg.setRTR(candle_frame_is_rtr(&frame));

            if (frame.flags & CANDLE_FLAG_FD) {
                msg.setFD(true);
            }
            if (frame.flags & CANDLE_FLAG_BRS) {
                msg.setBRS(true);
            }

            uint8_t dlc = candle_frame_dlc(&frame);
            uint8_t *data = candle_frame_data(&frame);
            msg.setLength(dlc);
            for (int i=0; i<dlc && i<64; i++) {
                msg.setByte(i, data[i]);
            }

            uint32_t dev_ts = candle_frame_timestamp_us(&frame) - _device->deviceTicksStart();
            uint64_t ts_us = _device->hostOffsetStart() + dev_ts;

            uint64_t us_since_start = _backend.getUsecsSinceMeasurementStart();
            if (us_since_start > 0x180000000) { // device timestamp overflow must have happend at least once
                ts_us += us_since_start & 0xFFFFFFFF00000000;
            }

            msg.setTimestamp(ts_us/1000000, ts_us % 1000000);
            msglist.append(msg);
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
    return _numRx;
}

int CandleApiInterface::getNumRxErrors()
{
    return 0;
}

int CandleApiInterface::getNumTxFrames()
{
    return _numTx;
}

int CandleApiInterface::getNumTxErrors()
{
    return _numTxErr;
}

int CandleApiInterface::getNumRxOverruns()
{
    return 0;
}

int CandleApiInterface::getNumTxDropped()
{
    return 0;
}

wstring CandleApiInterface::getPath() const
{
    return _device->path();
}

