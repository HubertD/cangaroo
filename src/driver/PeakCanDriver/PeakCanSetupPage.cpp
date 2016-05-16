#include "PeakCanSetupPage.h"
#include "ui_PeakCanSetupPage.h"

#include <core/Backend.h>
#include <core/MeasurementInterface.h>
#include <window/SetupDialog/SetupDialog.h>
#include "PeakCanInterface.h"

PeakCanSetupPage::PeakCanSetupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PeakCanSetupPage),
    _mi(0),
    _enable_ui_updates(true)
{
    ui->setupUi(this);

    connect(ui->cbListenOnly, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbAutoRestart, SIGNAL(stateChanged(int)), this, SLOT(updateUI()));
    connect(ui->cbBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateUI()));
}

PeakCanSetupPage::~PeakCanSetupPage()
{
    delete ui;
}

void PeakCanSetupPage::onSetupDialogCreated(SetupDialog &dlg)
{
    dlg.addPage(this);
    connect(&dlg, SIGNAL(onShowInterfacePage(SetupDialog&,MeasurementInterface*)), this, SLOT(onShowInterfacePage(SetupDialog&,MeasurementInterface*)));
}

void PeakCanSetupPage::onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi)
{
    PeakCanInterface *pci = 0;
    if (mi && (backend().getDriverName(mi->canInterface()) == "PCAN")) {
        _mi = mi;
         pci = (PeakCanInterface*)backend().getInterfaceById(_mi->canInterface());
    } else {
        return;
    }

    _enable_ui_updates = false;

    ui->laDriver->setText(backend().getDriverName(_mi->canInterface()));
    ui->laInterfaceName->setText(backend().getInterfaceName(_mi->canInterface()));

    ui->cbListenOnly->setChecked(_mi->isListenOnlyMode());
    ui->cbAutoRestart->setChecked(_mi->doAutoRestart());

    ui->cbBitrate->clear();
    foreach (CanTiming br, pci->getAvailableBitrates()) {
        ui->cbBitrate->addItem(QString::number(br.getBitrate()));
    }
    ui->cbBitrate->setCurrentText(QString::number(_mi->bitrate()));

    _enable_ui_updates = true;

    dlg.displayPage(this);
    updateUI();
}

void PeakCanSetupPage::updateUI()
{
    PeakCanInterface *pci = (PeakCanInterface*)backend().getInterfaceById(_mi->canInterface());

    if ((!_enable_ui_updates) || (pci==0)) {
        return;
    }

    _mi->setListenOnlyMode(ui->cbListenOnly->isChecked());
    _mi->setAutoRestart(ui->cbAutoRestart->isChecked());
    _mi->setBitrate(ui->cbBitrate->currentText().toInt());
}

Backend &PeakCanSetupPage::backend()
{
    return Backend::instance();
}
