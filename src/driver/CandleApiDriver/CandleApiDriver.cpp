/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CandleApiDriver.h"
#include "api/candle.h"

#include "CandleApiInterface.h"
#include "CandleApiDevice.h"
#include <driver/GenericCanSetupPage.h>

CandleApiDriver::CandleApiDriver(Backend &backend)
  : CanDriver(backend),
    setupPage(new GenericCanSetupPage(0))
{
    QObject::connect(&backend, SIGNAL(onSetupDialogCreated(SetupDialog&)), setupPage, SLOT(onSetupDialogCreated(SetupDialog&)));
}

CandleApiDriver::~CandleApiDriver()
{
    deleteAllDevices();
}

QString CandleApiDriver::getName()
{
    return "CandleAPI";
}

void CandleApiDriver::deleteAllDevices()
{
    qDeleteAll(_devices);
    _devices.clear();
}

bool CandleApiDriver::update()
{
    // Interfaces hold pointers into their device, so drop them before the
    // devices they reference.
    deleteAllInterfaces();
    deleteAllDevices();

    candle_list_handle clist;
    uint8_t num_devices;
    candle_handle dev;

    if (candle_list_scan(&clist)) {
        if (candle_list_length(clist, &num_devices)) {
            for (uint8_t i=0; i<num_devices; i++) {
                if (candle_dev_get(clist, i, &dev)) {
                    CandleApiDevice *device = new CandleApiDevice(dev);
                    _devices.append(device);

                    // One interface per physical CAN channel on the device.
                    uint8_t channels = device->channelCount();
                    for (uint8_t ch=0; ch<channels; ch++) {
                        addInterface(new CandleApiInterface(this, device, ch));
                    }
                }
            }
        }
        candle_list_free(clist);
    }

    return true;
}
