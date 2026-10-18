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
#include <QItemSelectionModel>
#include <QMenu>
#include <QFileDialog>
#include <QTreeWidget>
#include <QMessageBox>

#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <driver/CanInterface.h>
#include <driver/CanDriver.h>

#include "SetupDialogTreeModel.h"
#include "SetupDialogTreeItem.h"

#include "SelectCanInterfacesDialog.h"

SetupDialog::SetupDialog(Backend &backend, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    _backend(&backend),
    _isReflashNetworks(false),
    _currentNetwork(0)
{
    ui->setupUi(this);

    QIcon icon(":/assets/cangaroo.png");
    setWindowIcon(icon);

    _actionAddInterface = new QAction("Add...", this);
    _actionDeleteInterface = new QAction("Delete", this);
    _actionAddCanDb = new QAction("Add...", this);
    _actionDeleteCanDb = new QAction("Delete", this);
    _actionReloadCanDbs = new QAction("Reload", this);

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

    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenu(QPoint)));
    connect(ui->candbsTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(candbsTreeViewContextMenu(QPoint)));
    connect(ui->edNetworkName, SIGNAL(textChanged(QString)), this, SLOT(edNetworkNameChanged()));

    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(treeViewSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->candbsTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));
    connect(ui->interfacesTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));

    connect(ui->btReloadDbc, SIGNAL (released()), this, SLOT(executeReloadCanDbs()));
    connect(ui->btRefreshNetworks, SIGNAL(released()), this, SLOT(on_btRefreshNetworks_clicked()));

    connect(_actionAddCanDb, SIGNAL(triggered()), this, SLOT(executeAddCanDb()));
    connect(_actionDeleteCanDb, SIGNAL(triggered()), this, SLOT(executeDeleteCanDb()));

    connect(_actionAddInterface, SIGNAL(triggered()), this, SLOT(executeAddInterface()));
    connect(_actionDeleteInterface, SIGNAL(triggered()), this, SLOT(executeDeleteInterface()));


    emit backend.onSetupDialogCreated(*this);
}

SetupDialog::~SetupDialog()
{
    delete ui;
    delete model;
}

void SetupDialog::addPage(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
}

void SetupDialog::displayPage(QWidget *widget)
{
    ui->stackedWidget->setCurrentWidget(widget);
}

bool SetupDialog::showSetupDialog(MeasurementSetup &setup)
{
    _isReflashNetworks = false;

    model->load(setup);
    ui->treeView->expandAll();

    QModelIndex first = model->index(0, 0, QModelIndex());
    ui->treeView->setCurrentIndex(first);

    updateButtons();
    bool result = exec()==QDialog::Accepted;
    model->unload();
    return result;
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

    if (!item) {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
        updateButtons();
        return;
    }

    _currentNetwork = item->network;

    if (item->network) {
        ui->edNetworkName->setText(item->network->name());
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
                emit onShowInterfacePage(*this, item->intf);
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
                contextMenu.addAction(_actionReloadCanDbs);
                break;
            default:
                break;
        }
    }


    QPoint globalPos = ui->treeView->mapToGlobal(pos);
    contextMenu.exec(globalPos);
}

void SetupDialog::candbsTreeViewContextMenu(const QPoint &pos)
{
    QMenu contextMenu;
    QModelIndex index = ui->candbsTreeView->indexAt(pos);
    if (index.isValid()) {
        ui->candbsTreeView->setCurrentIndex(index);
        contextMenu.addAction(_actionReloadCanDbs);
        contextMenu.addAction(_actionDeleteCanDb);
    } else {
        contextMenu.addAction(_actionAddCanDb);
    }
    QPoint globalPos = ui->candbsTreeView->mapToGlobal(pos);
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

void SetupDialog::reloadCanDbs(const QModelIndex &index)
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    if (!item || !item->network) return;

    QStringList errors;
    if (!item->network->reloadCanDbs(_backend, &errors)) {
        QMessageBox::warning(this, tr("DBC Error"),
            tr("Failed to reload one or more DBC files:\n\n%1").arg(errors.join("\n")));
    }

    // Synchronize stale pointers in tree items
    SetupDialogTreeItem *root = (item->getType() == SetupDialogTreeItem::type_candb_root) ? item : item->getParentItem();
    if (root && root->getType() == SetupDialogTreeItem::type_candb_root) {
        for (int i=0; i < root->getChildCount(); ++i) {
            SetupDialogTreeItem *child = root->child(i);
            if (child->getType() == SetupDialogTreeItem::type_candb) {
                 // Find the updated pCanDb in the network by path
                 foreach (pCanDb updatedDb, root->network->_canDbs) {
                     if (updatedDb->getPath() == child->candb->getPath()) {
                         child->candb = updatedDb;
                         break;
                     }
                 }
            }
        }
    }

    model->dataChanged(index, index);
}

void SetupDialog::executeReloadCanDbs()
{
    reloadCanDbs(ui->treeView->selectionModel()->currentIndex());
}

void SetupDialog::addCanDb(const QModelIndex &parent, const QString &filename)
{
    // Check for duplicates
    if (_currentNetwork) {
        foreach (pCanDb existingDb, _currentNetwork->_canDbs) {
            if (existingDb->getPath() == filename) {
                QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this, tr("Duplicate DBC"),
                    tr("The file is already loaded:\n%1\n\nDo you want to reload it?").arg(filename),
                    QMessageBox::Yes | QMessageBox::No);

                if (reply == QMessageBox::Yes) {
                    // Find the index of the existing DBC in the model to remove it first
                    SetupDialogTreeItem *root = static_cast<SetupDialogTreeItem*>(parent.internalPointer());
                    if (root && root->getType() == SetupDialogTreeItem::type_candb_root) {
                        for (int i=0; i < root->getChildCount(); ++i) {
                            SetupDialogTreeItem *child = root->child(i);
                            if (child->getType() == SetupDialogTreeItem::type_candb && child->candb->getPath() == filename) {
                                model->deleteCanDb(model->indexOfItem(child));
                                break;
                            }
                        }
                    }

                    // Now load fresh
                    QString errorMsg;
                    pCanDb candb = _backend->loadDbc(filename, &errorMsg);
                    if (candb) {
                        model->addCanDb(parent, candb);
                    } else {
                        QMessageBox::critical(this, tr("Reload Failed"),
                            tr("Failed to reload DBC:\n%1\n\nReason: %2").arg(filename, errorMsg));
                    }
                }
                return;
            }
        }
    }

    QString errorMsg;
    pCanDb candb = _backend->loadDbc(filename, &errorMsg);
    if (candb) {
        model->addCanDb(parent, candb);
        if (!errorMsg.isEmpty()) {
            QMessageBox::warning(this, tr("DBC Warning"), errorMsg);
        }
    } else {
        QMessageBox::critical(this, tr("DBC Error"),
            tr("Failed to load DBC file:\n%1\n\nReason: %2").arg(filename, errorMsg));
    }
}

void SetupDialog::on_btAddDbc_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Load CAN Databases", "", "Vector DBC Files (*.dbc)");
    if (files.isEmpty()) return;

    QModelIndex parent = ui->treeView->selectionModel()->currentIndex();
    for (const QString &filename : files) {
        addCanDb(parent, filename);
    }
}

void SetupDialog::executeAddCanDb()
{
    on_btAddDbc_clicked();
}

void SetupDialog::executeDeleteCanDb()
{
    model->deleteCanDb(getSelectedIndex());
}

void SetupDialog::on_btRemoveDbc_clicked()
{
    model->deleteCanDb(ui->candbsTreeView->selectionModel()->currentIndex());
}

void SetupDialog::updateButtons()
{
    ui->btRemoveDbc->setEnabled(ui->candbsTreeView->selectionModel()->hasSelection());
    ui->btReloadDbc->setEnabled(ui->candbsTreeView->selectionModel()->hasSelection());

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

void SetupDialog::on_btRefreshNetworks_clicked()
{
    _backend->setDefaultSetup();
    model->load(_backend->getSetup());
    ui->treeView->expandAll();

    QModelIndex first = model->index(0, 0, QModelIndex());
    if (first.isValid()) {
        ui->treeView->setCurrentIndex(first);
    } else {
        ui->stackedWidget->setCurrentWidget(ui->emptyPage);
    }

    updateButtons();
    _isReflashNetworks = true;
}

bool SetupDialog::isReflashNetworks()
{
    return _isReflashNetworks;
}

