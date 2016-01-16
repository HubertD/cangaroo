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

#include "SetupDialog.h"
#include "ui_SetupDialog.h"
#include "ui_SocketCanInterfacePage.h"
#include <QItemSelectionModel>
#include <QMenu>
#include <QFileDialog>
#include <QTreeWidget>

#include <Backend.h>
#include <model/MeasurementSetup.h>
#include <driver/CanInterface.h>
#include <driver/CanDriver.h>
#include <driver/socketcan/SocketCanInterface.h>


#include "SetupDialogTreeModel.h"
#include "SetupDialogTreeItem.h"

#include "SelectCanInterfacesDialog.h"

SetupDialog::SetupDialog(Backend &backend, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    socketCanPageUI(new Ui::SocketCanInterfacePage),
    _socketCanPage(new QWidget(this)),
    _backend(&backend),
    _enableSocketCanUpdates(true),
    _currentNetwork(0)
{
    ui->setupUi(this);
    socketCanPageUI->setupUi(_socketCanPage);

    QIcon icon(":/assets/cangaroo.png");
    setWindowIcon(icon);

    _actionAddInterface = new QAction("Add...", this);
    _actionDeleteInterface = new QAction("Delete", this);
    _actionAddCanDb = new QAction("Add...", this);
    _actionDeleteCanDb = new QAction("Delete", this);

    model = new SetupDialogTreeModel(_backend, this);

    ui->treeView->setModel(model);
    ui->interfacesTreeView->setModel(model);
    ui->candbsTreeView->setModel(model);

    for (int i=0; i<model->columnCount(); i++) {
        ui->treeView->setColumnHidden(i, true);
        ui->interfacesTreeView->setColumnHidden(i, true);
        ui->candbsTreeView->setColumnHidden(i, true);
    }

    ui->treeView->setColumnHidden(SetupDialogTreeModel::column_device, false);

    ui->interfacesTreeView->setColumnHidden(SetupDialogTreeModel::column_device, false);
    ui->interfacesTreeView->setColumnHidden(SetupDialogTreeModel::column_driver, false);
    ui->interfacesTreeView->setColumnHidden(SetupDialogTreeModel::column_bitrate, false);

    ui->candbsTreeView->setColumnHidden(SetupDialogTreeModel::column_filename, false);
    ui->candbsTreeView->setColumnHidden(SetupDialogTreeModel::column_path, false);

    ui->stackedWidget->addWidget(_socketCanPage);


    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenu(QPoint)));
    connect(ui->edNetworkName, SIGNAL(textChanged(QString)), this, SLOT(edNetworkNameChanged()));

    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(treeViewSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->candbsTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));
    connect(ui->interfacesTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));



    connect(_actionAddCanDb, SIGNAL(triggered()), this, SLOT(executeAddCanDb()));
    connect(_actionDeleteCanDb, SIGNAL(triggered()), this, SLOT(executeDeleteCanDb()));

    connect(_actionAddInterface, SIGNAL(triggered()), this, SLOT(executeAddInterface()));
    connect(_actionDeleteInterface, SIGNAL(triggered()), this, SLOT(executeDeleteInterface()));

    connect(socketCanPageUI->cbSocketCanConfigured, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanListenOnly, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanTripleSampling, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanCanFD, SIGNAL(toggled(bool)), this, SLOT(updateSocketCanUI()));

    connect(socketCanPageUI->rbSocketCanAutomaticTiming, SIGNAL(toggled(bool)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->slSocketCanSamplePoint, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanFdBitrate, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->slSocketCanFdSamplePoint, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));

    connect(socketCanPageUI->spSocketCanTQ, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanPropSeg, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanPhaseSeg1, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanPhaseSeg2, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanSJW, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));

    connect(socketCanPageUI->spSocketCanFdTQ, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanFdPropSeg, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanFdPhaseSeg1, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanFdPhaseSeg2, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->cbSocketCanFdSJW, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));

    connect(socketCanPageUI->cbSocketCanRestart, SIGNAL(stateChanged(int)), this, SLOT(updateSocketCanUI()));
    connect(socketCanPageUI->spSocketCanRestartTime, SIGNAL(valueChanged(int)), this, SLOT(updateSocketCanUI()));
}

SetupDialog::~SetupDialog()
{
    delete ui;
    delete model;
}

bool SetupDialog::showSetupDialog(MeasurementSetup &setup)
{
    model->load(setup);
    ui->treeView->expandAll();
    updateButtons();
    return exec()==QDialog::Accepted;
}

void SetupDialog::treeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    (void) selected;
    (void) deselected;

    _currentNetwork = 0;

    if (selected.isEmpty()) {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
        updateButtons();
        return;
    }

    QModelIndex idx = selected.first().topLeft();
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem *>(idx.internalPointer());


    _currentNetwork = item->network;
    _currentInterface = item->intf;

    if (item->network) {
        ui->edNetworkName->setText(item->network->name());
    }

    if (item->intf) {
        loadInterface(*item->intf);
    }

    if (item) {
        switch (item->getType()) {

            case SetupDialogTreeItem::type_network:
                ui->stackedWidget->setCurrentWidget(ui->networkPage);
                break;

            case SetupDialogTreeItem::type_interface_root:
                ui->stackedWidget->setCurrentWidget(ui->interfacesPage);
                ui->interfacesTreeView->setRootIndex(getSelectedIndex());
                break;

            case SetupDialogTreeItem::type_interface:
                showInterfacePage(item);
                break;

            case SetupDialogTreeItem::type_candb_root:
                ui->stackedWidget->setCurrentWidget(ui->candbsPage);
                ui->candbsTreeView->setRootIndex(getSelectedIndex());
                break;

            default:
                ui->stackedWidget->setCurrentWidget(ui->emptyPage);
                break;


        }
    }

    updateButtons();
}

QModelIndex SetupDialog::getSelectedIndex()
{
    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    if (list.isEmpty()) {
        return QModelIndex();
    } else {
        return list.first();
    }
}

SetupDialogTreeItem *SetupDialog::getSelectedItem()
{
    const QModelIndex index = getSelectedIndex();
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem *>(index.internalPointer());
    return item;
}

void SetupDialog::treeViewContextMenu(const QPoint &pos)
{
    (void) pos;

    QMenu contextMenu;

    SetupDialogTreeItem *item = getSelectedItem();
    if (item) {
        switch (item->getType()) {
            case SetupDialogTreeItem::type_interface_root:
                contextMenu.addAction(_actionAddInterface);
                break;
            case SetupDialogTreeItem::type_interface:
                contextMenu.addAction(_actionDeleteInterface);
                break;
            case SetupDialogTreeItem::type_candb_root:
                contextMenu.addAction(_actionAddCanDb);
                break;
            case SetupDialogTreeItem::type_candb:
                contextMenu.addAction(_actionDeleteCanDb);
                break;
            default:
                break;
        }
    }


    QPoint globalPos = ui->treeView->mapToGlobal(pos);
    contextMenu.exec(globalPos);
}

void SetupDialog::edNetworkNameChanged()
{
    if (_currentNetwork) {
        _currentNetwork->setName(ui->edNetworkName->text());
        model->dataChanged(getSelectedIndex(), getSelectedIndex());
    }
}

void SetupDialog::addInterface(const QModelIndex &parent)
{
    SelectCanInterfacesDialog dlg(0);
    CanInterfaceIdList list;
    if (dlg.selectInterfaces(*_backend, list, _currentNetwork->getReferencedCanInterfaces())) {
        foreach (CanInterfaceId intf, list) {
            model->addInterface(parent, intf);
        }
    }

}

void SetupDialog::loadInterface(const MeasurementInterface &intf)
{
    _enableSocketCanUpdates = false;

    CanInterface *ci = _backend->getInterfaceById(intf.canInterface());
    if (ci && (ci->getDriver()->getName()=="SocketCAN")) {
        SocketCanInterface *sci = (SocketCanInterface*)ci;
        //_supportsTimingConfig = sci->supportsTimingConfiguration();
    }
    socketCanPageUI->laInterfaceDriver->setText(_backend->getDriverName(intf.canInterface()));
    socketCanPageUI->laInterfaceName->setText(_backend->getInterfaceName(intf.canInterface()));

    socketCanPageUI->cbSocketCanConfigured->setChecked(intf.doConfigure());
    socketCanPageUI->cbSocketCanListenOnly->setChecked(intf.isListenOnlyMode());
    socketCanPageUI->cbSocketCanTripleSampling->setChecked(intf.isTripleSampling());
    socketCanPageUI->cbSocketCanCanFD->setChecked(intf.isCanFD());

    socketCanPageUI->rbSocketCanAutomaticTiming->setChecked(intf.isSimpleTiming());
    socketCanPageUI->cbSocketCanBitrate->clear();
    socketCanPageUI->cbSocketCanBitrate->setCurrentText(QString::number(intf.bitrate()));
    socketCanPageUI->slSocketCanSamplePoint->setValue(intf.samplePoint());

    socketCanPageUI->cbSocketCanFdBitrate->clear();
    if (ci) {
        foreach (int br, ci->getAvailableFdBitrates()) {
            socketCanPageUI->cbSocketCanFdBitrate->addItem(QString::number(br));
        }
    }
    socketCanPageUI->cbSocketCanFdBitrate->setCurrentText(QString::number(intf.fdBitrate()));
    socketCanPageUI->slSocketCanFdSamplePoint->setValue(intf.fdSamplePoint());


    socketCanPageUI->rbSocketCanManualTiming->setChecked(!intf.isSimpleTiming());
    socketCanPageUI->spSocketCanTQ->setValue(intf.tq());
    socketCanPageUI->spSocketCanPropSeg->setValue(intf.propSeg());
    socketCanPageUI->spSocketCanPhaseSeg1->setValue(intf.phaseSeg1());
    socketCanPageUI->spSocketCanPhaseSeg2->setValue(intf.phaseSeg2());
    socketCanPageUI->cbSocketCanSJW->setChecked(intf.doSetSJW());
    socketCanPageUI->spSocketCanSJW->setValue(intf.SJW());
    socketCanPageUI->spSocketCanFdTQ->setValue(intf.fdTq());
    socketCanPageUI->spSocketCanFdPropSeg->setValue(intf.fdPropSeg());
    socketCanPageUI->spSocketCanFdPhaseSeg1->setValue(intf.fdPhaseSeg1());
    socketCanPageUI->spSocketCanFdPhaseSeg2->setValue(intf.fdPhaseSeg2());
    socketCanPageUI->cbSocketCanFdSJW->setChecked(intf.doSetFdSJW());
    socketCanPageUI->spSocketCanFdSJW->setValue(intf.fdSJW());

    socketCanPageUI->cbSocketCanRestart->setChecked(intf.doAutoRestart());
    socketCanPageUI->spSocketCanRestartTime->setValue(intf.autoRestartMs());

    _enableSocketCanUpdates = true;
    updateSocketCanUI();
}

void SetupDialog::updateSocketCanUI()
{

    if (!_enableSocketCanUpdates) {
        return;
    }


    //socketCanPageUI->wSocketCanTimingMode->setEnabled(_supportsTimingConfig);
    //socketCanPageUI->wSocketCanTiming->setEnabled(_supportsTimingConfig);

    bool doConfig = socketCanPageUI->cbSocketCanConfigured->isChecked();
    socketCanPageUI->wSocketCanOptions->setEnabled(doConfig);
    socketCanPageUI->wSocketCanTiming->setEnabled(doConfig);

    bool enableCanFd = socketCanPageUI->cbSocketCanCanFD->isChecked();
    socketCanPageUI->wSocketCanAutoTimingCanFd->setEnabled(enableCanFd);
    socketCanPageUI->wSocketCanFdManualTiming->setEnabled(enableCanFd);

    bool autoTiming = socketCanPageUI->rbSocketCanAutomaticTiming->isChecked();
    socketCanPageUI->stackedSocketCanTiming->setCurrentIndex(autoTiming ? 0 : 1);

    bool configSjw = socketCanPageUI->cbSocketCanSJW->isChecked();
    socketCanPageUI->spSocketCanSJW->setEnabled(configSjw);

    bool configFdSjw = socketCanPageUI->cbSocketCanFdSJW->isChecked();
    socketCanPageUI->spSocketCanFdSJW->setEnabled(configFdSjw);

    bool autoRestart = socketCanPageUI->cbSocketCanRestart->isChecked();
    socketCanPageUI->spSocketCanRestartTime->setEnabled(autoRestart);

    socketCanPageUI->laSocketCanSamplePoint->setText(QString().sprintf("%.1f%%", socketCanPageUI->slSocketCanSamplePoint->value() / 10.0));
    socketCanPageUI->laSocketCanFdSamplePoint->setText(QString().sprintf("%.1f%%", socketCanPageUI->slSocketCanFdSamplePoint->value() / 10.0));

    if (_currentInterface) {
        _currentInterface->setDoConfigure(socketCanPageUI->cbSocketCanConfigured->isChecked());
        _currentInterface->setListenOnlyMode(socketCanPageUI->cbSocketCanListenOnly->isChecked());
        _currentInterface->setTripleSampling(socketCanPageUI->cbSocketCanTripleSampling->isChecked());
        _currentInterface->setCanFD(socketCanPageUI->cbSocketCanCanFD->isChecked());
        _currentInterface->setSimpleTiming(socketCanPageUI->rbSocketCanAutomaticTiming->isChecked());

        _currentInterface->setBitrate(socketCanPageUI->cbSocketCanBitrate->currentText().toInt());
        _currentInterface->setSamplePoint(socketCanPageUI->slSocketCanSamplePoint->value());
        _currentInterface->setFdBitrate(socketCanPageUI->cbSocketCanFdBitrate->currentText().toInt());
        _currentInterface->setFdSamplePoint(socketCanPageUI->slSocketCanFdSamplePoint->value());

        _currentInterface->setTq(socketCanPageUI->spSocketCanTQ->value());
        _currentInterface->setPropSeg(socketCanPageUI->spSocketCanPropSeg->value());
        _currentInterface->setPhaseSeg1(socketCanPageUI->spSocketCanPhaseSeg1->value());
        _currentInterface->setPhaseSeg2(socketCanPageUI->spSocketCanPhaseSeg2->value());
        _currentInterface->setDoSetSJW(socketCanPageUI->cbSocketCanSJW->isChecked());

        _currentInterface->setFdTq(socketCanPageUI->spSocketCanFdTQ->value());
        _currentInterface->setFdPropSeg(socketCanPageUI->spSocketCanFdPropSeg->value());
        _currentInterface->setFdPhaseSeg1(socketCanPageUI->spSocketCanFdPhaseSeg1->value());
        _currentInterface->setFdPhaseSeg2(socketCanPageUI->spSocketCanFdPhaseSeg2->value());
        _currentInterface->setDoSetFdSJW(socketCanPageUI->cbSocketCanFdSJW->isChecked());

        _currentInterface->setAutoRestart(socketCanPageUI->cbSocketCanRestart->isChecked());
        _currentInterface->setAutoRestartMs(socketCanPageUI->spSocketCanRestartTime->value());
    }

}

void SetupDialog::executeAddInterface()
{
    addInterface(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::on_btAddInterface_clicked()
{
    addInterface(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::executeDeleteInterface()
{
    model->deleteInterface(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::on_btRemoveInterface_clicked()
{
    model->deleteInterface(ui->interfacesTreeView->selectionModel()->currentIndex());
}

void SetupDialog::showInterfacePage(SetupDialogTreeItem *item)
{
    (void) item;
    ui->stackedWidget->setCurrentWidget(_socketCanPage);
    updateSocketCanUI();
}

void SetupDialog::addCanDb(const QModelIndex &parent)
{
    QString filename = QFileDialog::getOpenFileName(this, "Load CAN Database", "", "Vector DBC Files (*.dbc)");
    if (!filename.isNull()) {
        pCanDb candb = _backend->loadDbc(filename);
        model->addCanDb(parent, candb);
    }
}

void SetupDialog::executeAddCanDb()
{
    addCanDb(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::on_btAddDatabase_clicked()
{
    addCanDb(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::executeDeleteCanDb()
{
    model->deleteCanDb(getSelectedIndex());
}

void SetupDialog::on_btRemoveDatabase_clicked()
{
    model->deleteCanDb(ui->candbsTreeView->selectionModel()->currentIndex());
}

void SetupDialog::updateButtons()
{
    ui->btRemoveDatabase->setEnabled(ui->candbsTreeView->selectionModel()->hasSelection());
    ui->btRemoveInterface->setEnabled(ui->interfacesTreeView->selectionModel()->hasSelection());

    SetupDialogTreeItem *item = getSelectedItem();
    ui->btRemoveNetwork->setEnabled(ui->treeView->selectionModel()->hasSelection() && item && (item->getType()==SetupDialogTreeItem::type_network));
}

void SetupDialog::on_btAddNetwork_clicked()
{
    QModelIndex idx = model->indexOfItem(model->addNetwork());
    ui->treeView->expand(idx);
    ui->treeView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
}

void SetupDialog::on_btRemoveNetwork_clicked()
{
    model->deleteNetwork(getSelectedIndex());
}

