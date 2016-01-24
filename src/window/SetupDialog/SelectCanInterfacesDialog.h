/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

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

#include <QDialog>
#include <QList>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

class Backend;

namespace Ui {
class SelectCanInterfacesDialog;
}

class SelectCanInterfacesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCanInterfacesDialog(QWidget *parent = 0);
    ~SelectCanInterfacesDialog();

    bool selectInterfaces(Backend &backend, CanInterfaceIdList &selectedInterfaces, const CanInterfaceIdList &excludeInterfaces);

private:
    Ui::SelectCanInterfacesDialog *ui;
};
