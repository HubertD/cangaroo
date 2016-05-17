#ifndef CANDLEAPIINTERFACE_H
#define CANDLEAPIINTERFACE_H

#include <driver/CanInterface.h>
#include <core/MeasurementInterface.h>
#include "api/candle.h"
#include "CandleApiTiming.h"
#include <windows.h>
#include <QList>

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

    virtual uint32_t getCapabilities();
    virtual QList<CanTiming> getAvailableBitrates();

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

    uint64_t _perfTicksPerSecond;
    uint64_t _perfCountStart;

    uint32_t _deviceTicksLastSync;
    uint64_t _tLastSync_us;

    candle_handle _handle;
    MeasurementInterface _settings;

    uint64_t _numRx;
    uint64_t _numTx;
    uint64_t _numTxErr;

    QList<CandleApiTiming> _timings;

    void syncTimestamp();
    void checkSyncTimestamp();

    bool setBitTiming(uint32_t bitrate, uint32_t samplePoint);
};

#endif // CANDLEAPIINTERFACE_H
