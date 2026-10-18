#include "GenericCanSetupPage.h"
#include "ui_GenericCanSetupPage.h"
#include <core/Backend.h>
#include <driver/CanInterface.h>
#include <core/MeasurementInterface.h>
#include <window/SetupDialog/SetupDialog.h>
#include <QList>
#include <QtAlgorithms>
#include <algorithm>

GenericCanSetupPage::GenericCanSetupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenericCanSetupPage),
    _mi(0),
    _enable_ui_updates(false)
{
    ui->setupUi(this);
    connect(ui->cbBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbSamplePoint, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbBitrateFD, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbSamplePointFD, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));

    connect(ui->cbConfigOS, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbListenOnly, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbOneShot, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbTripleSampling, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbAutoRestart, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));

    connect(ui->cbCustomBitrate, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbCustomFdBitrate, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));

    connect(ui->CustomBitrateSet, SIGNAL(textChanged(QString)), this, SLOT(updateUI()));
    connect(ui->CustomFdBitrateSet, SIGNAL(textChanged(QString)), this, SLOT(updateUI()));
}

GenericCanSetupPage::~GenericCanSetupPage()
{
    delete ui;
}

void GenericCanSetupPage::onSetupDialogCreated(SetupDialog &dlg)
{
    dlg.addPage(this);
    connect(&dlg, SIGNAL(onShowInterfacePage(SetupDialog&,MeasurementInterface*)), this, SLOT(onShowInterfacePage(SetupDialog&,MeasurementInterface*)));
}

void GenericCanSetupPage::onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi)
{
    _mi = mi;
    CanInterface *intf = backend().getInterfaceById(_mi->canInterface());

    _enable_ui_updates = false;

    ui->laDriver->setText(intf->getDriver()->getName());
    ui->laInterface->setText(intf->getName());
    ui->laInterfaceDetails->setText(intf->getDetailsStr());

    fillBitratesList(intf, _mi->bitrate());
    fillFdBitrate(intf, _mi->fdBitrate());
    fillSamplePointsForBitrate(intf, _mi->bitrate(), _mi->samplePoint());
    fillSamplePointsForFdBitrate(intf,_mi->fdBitrate(),_mi->fdSamplePoint());

    ui->cbConfigOS->setChecked(!_mi->doConfigure());
    ui->cbListenOnly->setChecked(_mi->isListenOnlyMode());
    ui->cbOneShot->setChecked(_mi->isOneShotMode());
    ui->cbTripleSampling->setChecked(_mi->isTripleSampling());
    ui->cbAutoRestart->setChecked(_mi->doAutoRestart());

    ui->cbCustomBitrate->setChecked(_mi->isCustomBitrate());
    ui->cbCustomFdBitrate->setChecked(_mi->isCustomFdBitrate());

    ui->CustomBitrateSet->setText(QString("%1").arg(_mi->customBitrate(), 6, 16,QLatin1Char('0')).toUpper());
    ui->CustomFdBitrateSet->setText(QString("%1").arg(_mi->customFdBitrate(), 6, 16,QLatin1Char('0')).toUpper());

    disenableUI(_mi->doConfigure());
    dlg.displayPage(this);

    _enable_ui_updates = true;
}

void GenericCanSetupPage::updateUI()
{
    if (_enable_ui_updates && (_mi!=0)) {
        CanInterface *intf = backend().getInterfaceById(_mi->canInterface());

        _mi->setDoConfigure(!ui->cbConfigOS->isChecked());
        _mi->setListenOnlyMode(ui->cbListenOnly->isChecked());
        _mi->setOneShotMode(ui->cbOneShot->isChecked());
        _mi->setTripleSampling(ui->cbTripleSampling->isChecked());
        _mi->setAutoRestart(ui->cbAutoRestart->isChecked());
        _mi->setBitrate(ui->cbBitrate->currentData().toUInt());
        _mi->setSamplePoint(ui->cbSamplePoint->currentData().toUInt());
        _mi->setFdBitrate(ui->cbBitrateFD->currentData().toUInt());
        _mi->setFdSamplePoint(ui->cbSamplePointFD->currentData().toUInt());

        _mi->setCustomBitrateEn(ui->cbCustomBitrate->isChecked());
        _mi->setCustomFdBitrateEn(ui->cbCustomFdBitrate->isChecked());

        _enable_ui_updates = false;

        if(ui->cbCustomBitrate->isChecked())
        {
            if(ui->CustomBitrateSet->text().length() == 6)
            {
                uint8_t div,seg1,seg2;
                uint32_t temp;
                uint32_t CustomBitrateSet;
                CustomBitrateSet = ui->CustomBitrateSet->text().toUpper().toUInt(NULL, 16);
                div =  CustomBitrateSet >> 16;
                seg1 = CustomBitrateSet >> 8;
                seg2 = CustomBitrateSet & 0xff;

                if(div == 0)
                {
                    div = 1;
                }

                if(seg1 < 2)
                {
                    seg1 = 2;
                }

                if(seg2 < 2)
                {
                    seg2 = 2;
                }

                if(seg2 > 128)
                {
                    seg2 = 128;
                }
                temp = div << 16;
                CustomBitrateSet = temp;
                temp = seg1 << 8;
                CustomBitrateSet |= temp;
                temp = seg2;
                CustomBitrateSet |= temp;

                _mi->setCustomBitrate(CustomBitrateSet);
                ui->CustomBitrateSet->setText(QString("%1").arg(CustomBitrateSet, 6, 16,QLatin1Char('0')).toUpper());
            }
            else
            {
                _mi->setCustomBitrate(0x023407);
            }
        }

        if(ui->cbCustomFdBitrate->isChecked())
        {
            if(ui->CustomFdBitrateSet->text().length() == 6)
            {
                uint8_t div,seg1,seg2;
                uint32_t temp;
                uint32_t CustomFdBitrateSet;
                CustomFdBitrateSet = ui->CustomFdBitrateSet->text().toUpper().toUInt(NULL, 16);
                div =  CustomFdBitrateSet >> 16;
                seg1 = CustomFdBitrateSet >> 8;
                seg2 = CustomFdBitrateSet & 0xff;

                if(div == 0)
                {
                    div = 1;
                }

                if(seg1 == 0)
                {
                    seg1 = 1;
                }

                if(seg2 == 0)
                {
                    seg2 = 1;
                }

                if(div > 32)
                {
                    div = 32;
                }

                if(seg1 > 32)
                {
                    seg1 = 32;
                }

                if(seg2 > 16)
                {
                    seg2 = 16;
                }

                temp = div << 16;
                CustomFdBitrateSet = temp;
                temp = seg1 << 8;
                CustomFdBitrateSet |= temp;
                temp = seg2;
                CustomFdBitrateSet |= temp;

                _mi->setCustomFdBitrate(CustomFdBitrateSet);
                ui->CustomFdBitrateSet->setText(QString("%1").arg(CustomFdBitrateSet, 6, 16,QLatin1Char('0')).toUpper());
            }
            else
            {
                _mi->setCustomFdBitrate(0x011508);
            }
        }

        disenableUI(_mi->doConfigure());
        fillSamplePointsForBitrate(
            intf,
            ui->cbBitrate->currentData().toUInt(),
            ui->cbSamplePoint->currentData().toUInt()
        );
        fillSamplePointsForFdBitrate(
            intf,
            ui->cbBitrateFD->currentData().toUInt(),
            ui->cbSamplePointFD->currentData().toUInt()
        );
        _enable_ui_updates = true;
    }
}

void GenericCanSetupPage::fillBitratesList(CanInterface *intf, unsigned selectedBitrate)
{
    QList<uint32_t> bitrates;
    for (CanTiming t : intf->getAvailableBitrates()) {
        if (!bitrates.contains(t.getBitrate())) {
            bitrates.append(t.getBitrate());
        }
    }
    std::sort(bitrates.begin(), bitrates.end());

    ui->cbBitrate->clear();
    for (uint32_t br : bitrates) {
        ui->cbBitrate->addItem(QString::number(br), br);
    }
    ui->cbBitrate->setCurrentText(QString::number(selectedBitrate));
}

void GenericCanSetupPage::fillSamplePointsForBitrate(CanInterface *intf, unsigned selectedBitrate, unsigned selectedSamplePoint)
{
    QList<uint32_t> samplePoints;
    for (CanTiming t : intf->getAvailableBitrates()) {
        if (t.getBitrate() == selectedBitrate) {
            if (!samplePoints.contains(t.getSamplePoint())) {
                samplePoints.append(t.getSamplePoint());
            }
        }
    }
    std::sort(samplePoints.begin(), samplePoints.end());

    ui->cbSamplePoint->clear();
    for (uint32_t sp : samplePoints) {
        ui->cbSamplePoint->addItem(CanTiming::getSamplePointStr(sp), sp);
    }
    ui->cbSamplePoint->setCurrentText(CanTiming::getSamplePointStr(selectedSamplePoint));
}


void GenericCanSetupPage::fillFdBitrate(CanInterface *intf, unsigned selectedBitrate)
{
    QList<uint32_t> fdBitrates;
    unsigned currentArbBitrate = ui->cbBitrate->currentData().toUInt();

    for (CanTiming t : intf->getAvailableBitrates()) {
        if (t.getBitrate() == currentArbBitrate) {
            if (t.isCanFD() && !fdBitrates.contains(t.getBitrateFD())) {
                fdBitrates.append(t.getBitrateFD());
            }
        }
    }
    std::sort(fdBitrates.begin(), fdBitrates.end());

    ui->cbBitrateFD->clear();
    for (uint32_t fd_br : fdBitrates) {
        ui->cbBitrateFD->addItem(QString::number(fd_br), fd_br);
    }
    
    int idx = ui->cbBitrateFD->findData(selectedBitrate);
    if (idx >= 0) {
        ui->cbBitrateFD->setCurrentIndex(idx);
    } else if (ui->cbBitrateFD->count() > 0) {
        ui->cbBitrateFD->setCurrentIndex(0);
    }
}

void GenericCanSetupPage::fillSamplePointsForFdBitrate(CanInterface *intf, unsigned selectedBitrate, unsigned selectedSamplePoint)
{
    QList<uint32_t> samplePoints;
    for (CanTiming t : intf->getAvailableBitrates()) {
        if (t.getBitrateFD() == selectedBitrate) {
            if (!samplePoints.contains(t.getSamplePointFD())) {
                samplePoints.append(t.getSamplePointFD());
            }
        }
    }
    std::sort(samplePoints.begin(), samplePoints.end());

    ui->cbSamplePointFD->clear();
    for (uint32_t sp : samplePoints) {
        ui->cbSamplePointFD->addItem(CanTiming::getSamplePointFDStr(sp), sp);
    }
    ui->cbSamplePointFD->setCurrentText(CanTiming::getSamplePointFDStr(selectedSamplePoint));
}

void GenericCanSetupPage::disenableUI(bool enabled)
{

    CanInterface *intf = backend().getInterfaceById(_mi->canInterface());
    uint32_t caps = intf->getCapabilities();

    ui->cbBitrate->setEnabled(!ui->cbCustomBitrate->isChecked());
    ui->cbSamplePoint->setEnabled(!ui->cbCustomBitrate->isChecked());
    ui->cbConfigOS->setEnabled(caps & CanInterface::capability_config_os);

    ui->cbBitrateFD->setEnabled(!ui->cbCustomFdBitrate->isChecked() && (caps & CanInterface::capability_canfd));
    ui->cbSamplePointFD->setEnabled(!ui->cbCustomFdBitrate->isChecked() && (caps & CanInterface::capability_canfd));
    ui->cbListenOnly->setEnabled(enabled && (caps & CanInterface::capability_listen_only));
    ui->cbOneShot->setEnabled(enabled && (caps & CanInterface::capability_one_shot));
    ui->cbTripleSampling->setEnabled(enabled && (caps & CanInterface::capability_triple_sampling));
    ui->cbAutoRestart->setEnabled(enabled && (caps & CanInterface::capability_auto_restart));

    ui->cbCustomBitrate->setEnabled(enabled && (caps & CanInterface::capability_custom_bitrate));
    ui->cbCustomFdBitrate->setEnabled(enabled && (caps & CanInterface::capability_custom_canfd_bitrate));

    ui->CustomBitrateSet->setEnabled(ui->cbCustomBitrate->isChecked());
    ui->CustomFdBitrateSet->setEnabled(ui->cbCustomFdBitrate->isChecked());
}

Backend &GenericCanSetupPage::backend()
{
    return Backend::instance();
}
