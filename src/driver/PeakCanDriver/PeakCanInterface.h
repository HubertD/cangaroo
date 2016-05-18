#pragma once

#include <driver/CanInterface.h>
#include <QElapsedTimer>

class PeakCanDriver;

class PeakCanInterface : public CanInterface
{
public:
    PeakCanInterface(PeakCanDriver *driver, uint32_t handle);
    virtual ~PeakCanInterface();
    void update();
    uint32_t getHandle() const;

    virtual QString getName() const;

    virtual void applyConfig(const MeasurementInterface &mi);

    virtual uint32_t getCapabilities();
    virtual int getBitrate();

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

private:
    uint32_t _handle;
    uint64_t _hostOffsetFirstFrame;
    uint64_t _peakOffsetFirstFrame;
    void *_autoResetEvent;

    struct {
        bool autoRestart;
        bool listenOnly;
        int bitrate;
    } _config;

    uint16_t calcBitrateMode(int bitrate);
    QString getErrorText(uint32_t status_code);

};
