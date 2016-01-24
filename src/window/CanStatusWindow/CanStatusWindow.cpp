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


#include "CanStatusWindow.h"
#include "ui_CanStatusWindow.h"

#include <QStringList>
#include <QTimer>
#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>

CanStatusWindow::CanStatusWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::CanStatusWindow),
    _backend(backend),
    _timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->treeWidget->setHeaderLabels(QStringList()
        << "Driver" << "Interface" << "State"
        << "Rx Frames" << "Rx Errors" << "Rx Overrun"
        << "Tx Frames" << "Tx Errors" << "Tx Dropped"
        << "# Warning" << "# Passive" << "# Bus Off" << " #Restarts"
    );
    ui->treeWidget->setColumnWidth(0, 80);
    ui->treeWidget->setColumnWidth(1, 70);

    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(beginMeasurement()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(endMeasurement()));
    connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
}

CanStatusWindow::~CanStatusWindow()
{
    delete ui;
}

void CanStatusWindow::beginMeasurement()
{
    ui->treeWidget->clear();
    foreach (CanInterfaceId ifid, backend().getInterfaceList()) {
        CanInterface *intf = backend().getInterfaceById(ifid);
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setData(0, Qt::UserRole, QVariant::fromValue((void*)intf));
        item->setText(column_driver, intf->getDriver()->getName());
        item->setText(column_interface, intf->getName());

        item->setTextAlignment(column_driver, Qt::AlignLeft);
        item->setTextAlignment(column_interface, Qt::AlignLeft);
        item->setTextAlignment(column_state, Qt::AlignCenter);
        for (int i=column_rx_frames; i<column_count; i++) {
            item->setTextAlignment(i, Qt::AlignRight);
        }

        ui->treeWidget->addTopLevelItem(item);
    }
    update();
    _timer->start(100);
}

void CanStatusWindow::endMeasurement()
{
    _timer->stop();
}

void CanStatusWindow::update()
{
    for (QTreeWidgetItemIterator it(ui->treeWidget); *it; ++it) {
        QTreeWidgetItem *item = *it;
        CanInterface *intf = (CanInterface *)item->data(0, Qt::UserRole).value<void *>();
        if (intf) {
            intf->updateStatistics();
            item->setText(column_state, intf->getStateText());
            item->setText(column_rx_frames, QString().number(intf->getNumRxFrames()));
            item->setText(column_rx_errors, QString().number(intf->getNumRxErrors()));
            item->setText(column_rx_overrun, QString().number(intf->getNumRxOverruns()));
            item->setText(column_tx_frames, QString().number(intf->getNumTxFrames()));
            item->setText(column_tx_errors, QString().number(intf->getNumTxErrors()));
            item->setText(column_tx_dropped, QString().number(intf->getNumTxDropped()));
        }
    }

}

Backend &CanStatusWindow::backend()
{
    return _backend;
}
