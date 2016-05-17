#include "CandleApiTiming.h"

CandleApiTiming::CandleApiTiming(uint32_t baseClk, uint32_t bitrate, uint32_t samplePoint, uint32_t brp, uint32_t phase_seg1, uint32_t phase_seg2)
  : _baseClk(baseClk),
    _bitrate(bitrate),
    _samplePoint(samplePoint)
{
    _timing.brp = brp;
    _timing.phase_seg1 = phase_seg1;
    _timing.phase_seg2 = phase_seg2;
    _timing.sjw = 1;
    _timing.prop_seg = 1;
}

uint32_t CandleApiTiming::getBaseClk() const
{
    return _baseClk;
}

uint32_t CandleApiTiming::getBitrate() const
{
    return _bitrate;
}

uint32_t CandleApiTiming::getSamplePoint() const
{
    return _samplePoint;
}

candle_bittiming_t CandleApiTiming::getTiming() const
{
    return _timing;
}
