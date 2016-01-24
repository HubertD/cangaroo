#include "PeakCanDriver.h"
#include "PeakCanInterface.h"
#include "PeakCanSetupPage.h"

#include <windows.h>
#include "pcan-basic-api/Include/PCANBasic.h"

PeakCanDriver::PeakCanDriver(Backend &backend)
  : CanDriver(backend),
    setupPage(new PeakCanSetupPage(0))
{
    QObject::connect(&backend, SIGNAL(onSetupDialogCreated(SetupDialog&)), setupPage, SLOT(onSetupDialogCreated(SetupDialog&)));
}

QString PeakCanDriver::getName()
{
    return "PCAN";
}

bool PeakCanDriver::update()
{
    uint32_t channel_status;
    for (uint32_t i=PCAN_USBBUS1; i<=PCAN_USBBUS8; i++) {
        TPCANStatus result = CAN_GetValue(i, PCAN_CHANNEL_CONDITION, &channel_status, sizeof(channel_status));
        // returns PCAN_CHANNEL_UNAVAILABLE, PCAN_CHANNEL_AVAILABLE, PCAN_CHANNEL_OCCUPIED, PCAN_CHANNEL_PCANVIEW
        if(result == PCAN_ERROR_OK) {
            if (channel_status!=PCAN_CHANNEL_UNAVAILABLE) {
                PeakCanInterface *intf = createOrUpdateInterface(i);
                intf->update();
            }
        } else {
            return false;
        }
    }
    return true;
}

PeakCanInterface *PeakCanDriver::createOrUpdateInterface(uint32_t hnd)
{
    foreach (CanInterface *intf, getInterfaces()) {
        PeakCanInterface *pcif = dynamic_cast<PeakCanInterface*>(intf);
        if (pcif->getHandle() == hnd) {
            return pcif;
        }
    }

    PeakCanInterface *pcif = new PeakCanInterface(this, hnd);
    addInterface(pcif);
    return pcif;
}
