#ifndef PEAKCANDRIVER_H
#define PEAKCANDRIVER_H

#include <Backend.h>
#include <driver/CanDriver.h>

class PeakCanInterface;

class PeakCanDriver : public CanDriver
{
public:
    PeakCanDriver(Backend &backend);

    virtual QString getName();
    virtual bool update();

private:
    PeakCanInterface *createOrUpdateInterface(uint32_t hnd);

};

#endif // PEAKCANDRIVER_H
