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


#ifndef SETUPDIALOGINTERFACEPAGE_H
#define SETUPDIALOGINTERFACEPAGE_H

#include <QWidget>

namespace Ui {
class SetupDialogInterfacePage;
}

class Backend;
class SetupDialog;
class MeasurementInterface;

class SetupDialogInterfacePage : public QWidget
{
    Q_OBJECT

public:
    explicit SetupDialogInterfacePage(QWidget *parent, Backend &backend);
    ~SetupDialogInterfacePage();

public slots:
    void onSetupDialogCreated(SetupDialog &dlg);
    void onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi);

private slots:
    void updateUI();

private:
    Ui::SetupDialogInterfacePage *ui;
    Backend &_backend;
    bool _enable_ui_updates;
    MeasurementInterface *_mi;

    Backend &backend();
};

#endif // SETUPDIALOGINTERFACEPAGE_H
