#ifndef CANDLEAPIINTERFACE_H
#define CANDLEAPIINTERFACE_H

#include <driver/CanInterface.h>
#include "api/candle.h"

class CandleApiDriver;

using namespace std;

class CandleApiInterface : public CanInterface
{
public:
    CandleApiInterface(CandleApiDriver *driver, candle_handle handle);
    virtual ~CandleApiInterface();
    virtual QString getName() const;
    virtual void applyConfig(const MeasurementInterface &mi);

    virtual int getBitrate();

    virtual QList<int> getAvailableBitrates();
    virtual QList<int> getAvailableFdBitrates();

    virtual void open();
    virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(CanMessage &msg, unsigned int timeout_ms);

    virtual bool updateStatistics();
    virtual uint32_t getState();
    virtual int getNumRxFrames();
    virtual int getNumRxErrors();
    virtual int getNumTxFrames();
    virtual int getNumTxErrors();
    virtual int getNumRxOverruns();
    virtual int getNumTxDropped();

    wstring getPath();

    void update(candle_handle dev);

private:
    candle_handle _handle;
    uint32_t _bitrate;
    uint64_t _start_devtime_us;
    uint64_t _start_systime_us;

};

#endif // CANDLEAPIINTERFACE_H
