#include "CandleApiSetupPage.h"
#include "ui_candleapisetuppage.h"
#include <core/Backend.h>
#include <core/MeasurementInterface.h>
#include <window/SetupDialog/SetupDialog.h>
#include "CandleApiInterface.h"

CandleApiSetupPage::CandleApiSetupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CandleApiSetupPage),
    _mi(0)
{
    ui->setupUi(this);
}

CandleApiSetupPage::~CandleApiSetupPage()
{
    delete ui;
}

void CandleApiSetupPage::onSetupDialogCreated(SetupDialog &dlg)
{
    dlg.addPage(this);
    connect(&dlg, SIGNAL(onShowInterfacePage(SetupDialog&,MeasurementInterface*)), this, SLOT(onShowInterfacePage(SetupDialog&,MeasurementInterface*)));
}

void CandleApiSetupPage::onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi)
{
    CandleApiInterface *pci = 0;
    if (mi && (backend().getDriverName(mi->canInterface()) == "CandleAPI")) {
        _mi = mi;
         pci = (CandleApiInterface*)backend().getInterfaceById(_mi->canInterface());
    } else {
        return;
    }

    //_enable_ui_updates = false;

    ui->laDriver->setText(backend().getDriverName(_mi->canInterface()));
    ui->laInterfaceName->setText(backend().getInterfaceName(_mi->canInterface()));

    //ui->cbListenOnly->setChecked(_mi->isListenOnlyMode());
    //ui->cbAutoRestart->setChecked(_mi->doAutoRestart());

    ui->cbBitrate->clear();
    foreach (CanTiming br, pci->getAvailableBitrates()) {
        ui->cbBitrate->addItem(QString::number(br.getBitrate()));
    }
    ui->cbBitrate->setCurrentText(QString::number(_mi->bitrate()));

    //_enable_ui_updates = true;

    dlg.displayPage(this);
    //updateUI();

}

Backend &CandleApiSetupPage::backend()
{
    return Backend::instance();
}
