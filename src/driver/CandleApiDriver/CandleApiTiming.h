#ifndef CANDLEAPITIMING_H
#define CANDLEAPITIMING_H

#include "api/candle.h"

class CandleApiTiming
{
public:
    CandleApiTiming(
        uint32_t baseClk,
        uint32_t bitrate,
        uint32_t samplePoint,
        uint32_t brp,
        uint32_t phase_seg1,
        uint32_t phase_seg2
    );

    uint32_t getBaseClk() const;
    uint32_t getBitrate() const;
    uint32_t getSamplePoint() const;
    candle_bittiming_t getTiming() const;

private:
    uint32_t _baseClk;
    uint32_t _bitrate;
    uint32_t _samplePoint;
    candle_bittiming_t _timing;

};

#endif // CANDLEAPITIMING_H
