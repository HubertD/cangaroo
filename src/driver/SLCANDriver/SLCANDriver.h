/*

  Copyright (c) 2015, 2016 Hubert Denkmair

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

#pragma once

#include <QString>
#include <core/Backend.h>
#include <driver/CanDriver.h>

class SLCANInterface;
class SetupDialogInterfacePage;
class GenericCanSetupPage;

class SLCANDriver: public CanDriver {
public:
    SLCANDriver(Backend &backend);
    virtual ~SLCANDriver();

    virtual QString getName();
    virtual bool update();

private:
    SLCANInterface *createOrUpdateInterface(int index, QString name, bool fd_support, uint32_t manufacturer);
    GenericCanSetupPage *setupPage;
    uint32_t _manufacturer;
};
