/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

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

#include "ReplayWindow.h"
#include "ui_ReplayWindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <core/CanLogParser.h>
#include <driver/CanInterface.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>

ReplayWindow::ReplayWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::ReplayWindow),
    _backend(backend),
    _replayer(new CanReplayer(this))
{
    ui->setupUi(this);

    refreshInterfaces();

    // Connections to replayer
    // Note: UI slots starting with "on_XXX" are auto-connected by setupUi()

    connect(_replayer, &CanReplayer::progressUpdated, this, &ReplayWindow::onProgressUpdated);
    connect(_replayer, &CanReplayer::replayFinished, this, &ReplayWindow::onReplayFinished);

    connect(ui->spinSpeed, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [=](double v){ _replayer->setPlaybackSpeed(v); });
    connect(ui->chkOverrideTiming, &QCheckBox::stateChanged, this, [=](int state){ _replayer->setOverrideTiming(state == Qt::Checked, ui->spinIntervalMs->value()); });
    connect(ui->spinIntervalMs, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int v){ _replayer->setOverrideTiming(ui->chkOverrideTiming->isChecked(), v); });

    _replayer->setTraceBuffer(_backend.getTrace());
    
    // Connect to backend signals so the interface list updates automatically
    connect(&backend, &Backend::onSetupChanged, this, &ReplayWindow::refreshInterfaces);
    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(refreshInterfaces()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(refreshInterfaces()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(on_btnPause_clicked()));

    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
}

ReplayWindow::~ReplayWindow()
{
    if (_replayer->isRunning()) {
        _replayer->stopReplay();
        _replayer->wait();
    }
    delete ui;
}

bool ReplayWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "ReplayWindow");
    return true;
}

bool ReplayWindow::loadXML(Backend &backend, QDomElement &el)
{
    return ConfigurableWidget::loadXML(backend, el);
}

void ReplayWindow::refreshInterfaces()
{
    ui->cbInterface->blockSignals(true);
    ui->cbInterface->clear();
    
    MeasurementSetup &setup = _backend.getSetup();
    foreach (MeasurementNetwork *network, setup.getNetworks()) {
        foreach (MeasurementInterface *mi, network->interfaces()) {
            CanInterfaceId ifid = mi->canInterface();
            CanInterface *intf = _backend.getInterfaceById(ifid);
            if (intf) {
                QString name = network->name() + ": " + intf->getName();
                ui->cbInterface->addItem(name, QVariant(ifid));
            }
        }
    }
    if (ui->cbInterface->count() > 0 && ui->cbInterface->currentIndex() == -1) {
        ui->cbInterface->setCurrentIndex(0);
    }
    ui->cbInterface->blockSignals(false);
}

void ReplayWindow::on_btnBrowse_clicked()
{
    QString filters("All logs (*.asc *.candump *.txt);;Vector ASC (*.asc);;Linux candump (*.candump *.txt);;All files (*.*)");
    QString filename = QFileDialog::getOpenFileName(this, tr("Open CAN Log File"), QDir::currentPath(), filters);

    if (!filename.isEmpty()) {
        loadLogFile(filename);
    }
}

void ReplayWindow::loadLogFile(const QString& filename)
{
    _loadedMessages.clear();
    bool success = false;

    if (filename.endsWith(".candump", Qt::CaseInsensitive)) {
        success = CanLogParser::parseCanDump(filename, _loadedMessages);
    } else if (filename.endsWith(".asc", Qt::CaseInsensitive)) {
        success = CanLogParser::parseVectorAsc(filename, _loadedMessages);
    } else {
        // Fallback for custom extensions, try candump first, then ASC
        success = CanLogParser::parseCanDump(filename, _loadedMessages);
        if (!success || _loadedMessages.isEmpty()) {
            _loadedMessages.clear();
            success = CanLogParser::parseVectorAsc(filename, _loadedMessages);
        }
    }

    if (success && !_loadedMessages.isEmpty()) {
        ui->txtFilePath->setText(filename);
        ui->progressBar->setMaximum(_loadedMessages.size());
        ui->progressBar->setValue(0);
        ui->lblStatus->setText(tr("Loaded %1 messages.").arg(_loadedMessages.size()));
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Failed to load or parse the selected log file."));
        ui->txtFilePath->clear();
        ui->lblStatus->setText(tr("Ready."));
    }
}

void ReplayWindow::on_btnPlay_clicked()
{
    if (_loadedMessages.isEmpty()) return;

    if (_replayer->isRunning()) {
        // Just resume if it's paused
        _replayer->resumeReplay();
    } else {
        // Start fresh
        int idx = ui->cbInterface->currentIndex();
        if (idx < 0) return;

        CanInterfaceId id = ui->cbInterface->itemData(idx).toUInt();
        CanInterface* target = _backend.getInterfaceById(id);
        if (!target) return;

        _replayer->setMessages(_loadedMessages);
        _replayer->setTargetInterface(target);
        _replayer->setLooping(ui->chkLoop->isChecked());
        _replayer->start();
        
        ui->progressBar->setMaximum(_loadedMessages.size());
        ui->progressBar->setValue(0);
    }

    ui->lblStatus->setText(tr("Playing..."));
    ui->btnPlay->setEnabled(false);
    ui->btnPause->setEnabled(true);
    ui->btnStop->setEnabled(true);
}

void ReplayWindow::on_btnPause_clicked()
{
    if (_replayer->isRunning()) {
        _replayer->pauseReplay();
        ui->lblStatus->setText(tr("Paused."));
        ui->btnPlay->setEnabled(true);
        ui->btnPause->setEnabled(false);
    }
}

void ReplayWindow::on_btnStop_clicked()
{
    if (_replayer->isRunning()) {
        _replayer->stopReplay();
        _replayer->wait();
    }
    
    ui->progressBar->setValue(0);
    ui->lblStatus->setText(tr("Stopped."));
    ui->btnPlay->setEnabled(true);
    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
}

void ReplayWindow::on_chkLoop_stateChanged(int)
{
    _replayer->setLooping(ui->chkLoop->isChecked());
}

void ReplayWindow::on_cbInterface_currentIndexChanged(int)
{
    // Update target interface immediately if running
    if (_replayer->isRunning()) {
        int idx = ui->cbInterface->currentIndex();
        if (idx >= 0) {
            CanInterfaceId id = ui->cbInterface->itemData(idx).toUInt();
            CanInterface* target = _backend.getInterfaceById(id);
            if (target) {
                _replayer->setTargetInterface(target);
            }
        }
    }
}

void ReplayWindow::onProgressUpdated(int index, int total)
{
    ui->progressBar->setValue(index);
    ui->lblStatus->setText(tr("Playing... (%1 / %2)").arg(index).arg(total));
}

void ReplayWindow::onReplayFinished()
{
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->lblStatus->setText(tr("Finished playback."));
    ui->btnPlay->setEnabled(true);
    ui->btnPause->setEnabled(false);
    ui->btnStop->setEnabled(false);
}
