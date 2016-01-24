#pragma once

#include <core/Backend.h>
#include <driver/CanDriver.h>

class PeakCanInterface;
class PeakCanSetupPage;

class PeakCanDriver : public CanDriver
{
public:
    PeakCanDriver(Backend &backend);

    virtual QString getName();
    virtual bool update();

private:
    PeakCanInterface *createOrUpdateInterface(uint32_t hnd);
    PeakCanSetupPage *setupPage;

};
