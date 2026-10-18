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
#include <QDateTime>
#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>
#include <driver/CanInterface.h>

CanStatusWindow::CanStatusWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::CanStatusWindow),
    _backend(backend),
    _timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->treeWidget->setHeaderLabels(QStringList()
                                    << tr("Driver") << tr("Interface") << tr("State")
                                    << tr("Rx Frames") << tr("Rx Errors") << tr("Rx Overrun")
                                    << tr("Tx Frames") << tr("Tx Errors") << tr("Tx Dropped")
                                    << tr("Load (%)") << tr("Bits")
        // << "# Warning" << "# Passive" << "# Bus Off" << " #Restarts"
    );
    // Driver width
    ui->treeWidget->setColumnWidth(0, 100);
    // Interface width
    ui->treeWidget->setColumnWidth(1, 110);
    // Bus Load width
    ui->treeWidget->setColumnWidth(column_bus_load, 90);

    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(beginMeasurement()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(endMeasurement()));
    connect(&backend, SIGNAL(onClearTraceRequested()), this, SLOT(clearStatistics()));
    connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
}

CanStatusWindow::~CanStatusWindow()
{
    delete ui;
}

void CanStatusWindow::beginMeasurement()
{
    ui->treeWidget->clear();
    _lastStats.clear();
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
    update();
    _timer->stop();
}

void CanStatusWindow::clearStatistics()
{
    // Reset statistics in all active interfaces
    foreach (CanInterfaceId ifid, backend().getInterfaceList()) {
        CanInterface *intf = backend().getInterfaceById(ifid);
        if (intf) {
            intf->resetStatistics();
            intf->updateStatistics();
        }
    }
    update();
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
            
            // Calculate Bus Load (%)
            qint64 now = QDateTime::currentMSecsSinceEpoch();
            uint64_t currentBits = intf->getNumBits();
             if (_lastStats.contains(intf)) {
                 InterfaceStats &ls = _lastStats[intf];
                 qint64 dt = now - ls.lastTime;
                 if (dt >= 500) { // Update load every ~500ms for stability
                     uint64_t dbits = currentBits - ls.lastBits;
                     unsigned bitrate = intf->getBitrate();
                     if (bitrate > 0) {
                         double load = (double)dbits * 1000.0 / (double)bitrate / (double)dt * 100.0;
                         if (load > 100.0) load = 100.0;
                         item->setText(column_bus_load, QString("%1%").arg(load, 0, 'f', 1));
                     } else {
                         item->setText(column_bus_load, "---");
                     }
                     ls.lastBits = currentBits;
                     ls.lastTime = now;
                 }
             } else {
                 _lastStats[intf] = { currentBits, now };
                 item->setText(column_bus_load, "0.0%");
             }
             item->setText(column_bits, QString::number(currentBits));
        }
    }

}

Backend &CanStatusWindow::backend()
{
    return _backend;
}

QSize CanStatusWindow::sizeHint() const
{
    return QSize(1200, 600);
}
