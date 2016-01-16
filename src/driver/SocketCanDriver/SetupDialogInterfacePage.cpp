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


#include "SetupDialogInterfacePage.h"
#include "ui_SetupDialogInterfacePage.h"

#include <Backend.h>
#include <model/MeasurementInterface.h>
#include <window/SetupDialog/SetupDialog.h>
#include "SocketCanInterface.h"

SetupDialogInterfacePage::SetupDialogInterfacePage(QWidget *parent, Backend &backend) :
    QWidget(parent),
    ui(new Ui::SetupDialogInterfacePage),
    _backend(backend),
    _enable_ui_updates(true)
{
    ui->setupUi(this);


    connect(ui->cbConfigure, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbListenOnly, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbTripleSampling, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbCanFD, SIGNAL(toggled(bool)), this, SLOT(updateUI()));

    connect(ui->rbTimingSimple, SIGNAL(toggled(bool)), this, SLOT(updateUI()));
    connect(ui->cbBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    connect(ui->slSamplePoint, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbFdBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    connect(ui->slFdSamplePoint, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));

    connect(ui->spTQ, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spPropSeg, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spPhaseSeg1, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spPhaseSeg2, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbSJW, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));

    connect(ui->spFdTQ, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spFdPropSeg, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spFdPhaseSeg1, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->spFdPhaseSeg2, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbFdSJW, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));

    connect(ui->cbAutoRestart, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->spAutoRestartTime, SIGNAL(valueChanged(int)), this, SLOT(updateUI()));
}

SetupDialogInterfacePage::~SetupDialogInterfacePage()
{
    delete ui;
}

Backend &SetupDialogInterfacePage::backend()
{
    return _backend;
}

void SetupDialogInterfacePage::onSetupDialogCreated(SetupDialog &dlg)
{
    dlg.addPage(this);
    connect(&dlg, SIGNAL(onShowInterfacePage(SetupDialog&,MeasurementInterface*)), this, SLOT(onShowInterfacePage(SetupDialog&,MeasurementInterface*)));
}

void SetupDialogInterfacePage::onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi)
{
    SocketCanInterface *sci;
    if (mi && (backend().getDriverName(mi->canInterface()) == "SocketCAN")) {
        _mi = mi;
         sci = (SocketCanInterface*)backend().getInterfaceById(_mi->canInterface());
    } else {
        return;
    }

    _enable_ui_updates = false;

    ui->laDriver->setText(backend().getDriverName(_mi->canInterface()));
    ui->laInterfaceName->setText(backend().getInterfaceName(_mi->canInterface()));

    ui->cbConfigure->setChecked(_mi->doConfigure());
    ui->cbListenOnly->setChecked(_mi->isListenOnlyMode());
    ui->cbTripleSampling->setChecked(_mi->isTripleSampling());
    ui->cbCanFD->setChecked(_mi->isCanFD());

    ui->rbTimingSimple->setChecked(_mi->isSimpleTiming());
    ui->cbBitrate->clear();
    foreach (int br, sci->getAvailableBitrates()) {
        ui->cbBitrate->addItem(QString::number(br));
    }
    ui->cbBitrate->setCurrentText(QString::number(_mi->bitrate()));
    ui->slSamplePoint->setValue(_mi->samplePoint());

    ui->cbFdBitrate->clear();
    foreach (int br, sci->getAvailableFdBitrates()) {
        ui->cbFdBitrate->addItem(QString::number(br));
    }
    ui->cbFdBitrate->setCurrentText(QString::number(_mi->fdBitrate()));
    ui->slFdSamplePoint->setValue(_mi->fdSamplePoint());


    ui->rbTimingRegisters->setChecked(!_mi->isSimpleTiming());

    ui->spTQ->setValue(_mi->tq());
    ui->spPropSeg->setValue(_mi->propSeg());
    ui->spPhaseSeg1->setValue(_mi->phaseSeg1());
    ui->spPhaseSeg2->setValue(_mi->phaseSeg2());
    ui->cbSJW->setChecked(_mi->doSetSJW());
    ui->spSJW->setValue(_mi->SJW());

    ui->spFdTQ->setValue(_mi->fdTq());
    ui->spFdPropSeg->setValue(_mi->fdPropSeg());
    ui->spFdPhaseSeg1->setValue(_mi->fdPhaseSeg1());
    ui->spFdPhaseSeg2->setValue(_mi->fdPhaseSeg2());
    ui->cbFdSJW->setChecked(_mi->doSetFdSJW());
    ui->spFdSJW->setValue(_mi->fdSJW());

    ui->cbAutoRestart->setChecked(_mi->doAutoRestart());
    ui->spAutoRestartTime->setValue(_mi->autoRestartMs());

    _enable_ui_updates = true;

    dlg.displayPage(this);
    updateUI();
}

void SetupDialogInterfacePage::updateUI()
{
    if (!_enable_ui_updates) {
        return;
    }


    //ui->wSocketCanTimingMode->setEnabled(_supportsTimingConfig);
    //ui->wSocketCanTiming->setEnabled(_supportsTimingConfig);

    bool doConfig = ui->cbConfigure->isChecked();
    ui->wOptions->setEnabled(doConfig);
    ui->wTiming->setEnabled(doConfig);

    bool enableCanFd = ui->cbCanFD->isChecked();
    ui->wAutoTimingCanFd->setEnabled(enableCanFd);
    ui->wFdManualTiming->setEnabled(enableCanFd);

    bool autoTiming = ui->rbTimingSimple->isChecked();
    ui->stackedTiming->setCurrentIndex(autoTiming ? 0 : 1);

    bool configSjw = ui->cbSJW->isChecked();
    ui->spSJW->setEnabled(configSjw);

    bool configFdSjw = ui->cbFdSJW->isChecked();
    ui->spFdSJW->setEnabled(configFdSjw);

    bool autoRestart = ui->cbAutoRestart->isChecked();
    ui->spAutoRestartTime->setEnabled(autoRestart);

    ui->laSamplePoint->setText(QString().sprintf("%.1f%%", ui->slSamplePoint->value() / 10.0));
    ui->laFdSamplePoint->setText(QString().sprintf("%.1f%%", ui->slFdSamplePoint->value() / 10.0));

    _mi->setDoConfigure(ui->cbConfigure->isChecked());
    _mi->setListenOnlyMode(ui->cbListenOnly->isChecked());
    _mi->setTripleSampling(ui->cbTripleSampling->isChecked());
    _mi->setCanFD(ui->cbCanFD->isChecked());
    _mi->setSimpleTiming(ui->rbTimingSimple->isChecked());

    _mi->setBitrate(ui->cbBitrate->currentText().toInt());
    _mi->setSamplePoint(ui->slSamplePoint->value());
    _mi->setFdBitrate(ui->cbFdBitrate->currentText().toInt());
    _mi->setFdSamplePoint(ui->slFdSamplePoint->value());

    _mi->setTq(ui->spTQ->value());
    _mi->setPropSeg(ui->spPropSeg->value());
    _mi->setPhaseSeg1(ui->spPhaseSeg1->value());
    _mi->setPhaseSeg2(ui->spPhaseSeg2->value());
    _mi->setDoSetSJW(ui->cbSJW->isChecked());

    _mi->setFdTq(ui->spFdTQ->value());
    _mi->setFdPropSeg(ui->spFdPropSeg->value());
    _mi->setFdPhaseSeg1(ui->spFdPhaseSeg1->value());
    _mi->setFdPhaseSeg2(ui->spFdPhaseSeg2->value());
    _mi->setDoSetFdSJW(ui->cbFdSJW->isChecked());

    _mi->setAutoRestart(ui->cbAutoRestart->isChecked());
    _mi->setAutoRestartMs(ui->spAutoRestartTime->value());

}

