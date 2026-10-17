#ifndef CANDLEAPIINTERFACE_H
#define CANDLEAPIINTERFACE_H

#include <driver/CanInterface.h>
#include <core/MeasurementInterface.h>
#include "api/candle.h"
#include "CandleApiTiming.h"
#include "CandleApiDevice.h"
#include <windows.h>
#include <QList>

class CandleApiDriver;

using namespace std;

class CandleApiInterface : public CanInterface
{
    Q_OBJECT
public:
    CandleApiInterface(CandleApiDriver *driver, CandleApiDevice *device, uint8_t channel);
    virtual ~CandleApiInterface();

    virtual QString getName() const;
    virtual QString getDetailsStr() const;

    virtual void applyConfig(const MeasurementInterface &mi);

    virtual unsigned getBitrate();

    virtual uint32_t getCapabilities();
    virtual QList<CanTiming> getAvailableBitrates();

    virtual void open();
    virtual bool isOpen();
    virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms);

    virtual bool updateStatistics();
    virtual uint32_t getState();
    virtual int getNumRxFrames();
    virtual int getNumRxErrors();
    virtual int getNumTxFrames();
    virtual int getNumTxErrors();
    virtual int getNumRxOverruns();
    virtual int getNumTxDropped();

    wstring getPath() const;

private:

    bool _isOpen;

    CandleApiDevice *_device;
    uint8_t _ch;
    MeasurementInterface _settings;
    Backend &_backend;

    uint64_t _numRx;
    uint64_t _numTx;
    uint64_t _numTxErr;

    QList<CandleApiTiming> _timings;

    bool setBitTiming(uint32_t bitrate, uint32_t samplePoint, uint32_t fdBitrate, uint32_t fdSamplePoint, bool useFD);
};

#endif // CANDLEAPIINTERFACE_H
