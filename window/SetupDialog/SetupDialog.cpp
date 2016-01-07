#include "SetupDialog.h"
#include "ui_SetupDialog.h"

#include <QItemSelectionModel>
#include <QMenu>
#include <QFileDialog>
#include <QTreeWidget>

#include <parser/dbc/DbcParser.h>
#include <model/MeasurementSetup.h>
#include <drivers/CanInterface.h>
#include <drivers/CanInterfaceProvider.h>

#include "SetupDialogTreeModel.h"
#include "SetupDialogTreeItem.h"

#include "SelectCanInterfacesDialog.h"

SetupDialog::SetupDialog(Backend &backend, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    _backend(&backend),
    _currentNetwork(0)
{
    ui->setupUi(this);
    QIcon icon(":/assets/icon.png");
    setWindowIcon(icon);

    _actionAddInterface = new QAction("Add...", this);
    _actionDeleteInterface = new QAction("Delete", this);
    _actionAddCanDb = new QAction("Add...", this);
    _actionDeleteCanDb = new QAction("Delete", this);

    model = new SetupDialogTreeModel(this);

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
    connect(ui->edNetworkName, SIGNAL(textChanged(QString)), this, SLOT(edNetworkNameChanged()));
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(treeViewSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->candbsTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));
    connect(ui->interfacesTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateButtons()));

    connect(_actionAddCanDb, SIGNAL(triggered()), this, SLOT(executeAddCanDb()));
    connect(_actionDeleteCanDb, SIGNAL(triggered()), this, SLOT(executeDeleteCanDb()));

    connect(_actionAddInterface, SIGNAL(triggered()), this, SLOT(executeAddInterface()));
    connect(_actionDeleteInterface, SIGNAL(triggered()), this, SLOT(executeDeleteInterface()));
}

SetupDialog::~SetupDialog()
{
    delete ui;
    delete model;
}

MeasurementSetup *SetupDialog::showSetupDialog(MeasurementSetup &setup)
{
    MeasurementSetup *mySetup = new MeasurementSetup(0);
    mySetup->cloneFrom(setup);

    model->load(mySetup);
    ui->treeView->expandAll();

    updateButtons();

    if (exec()==QDialog::Accepted) {
        return mySetup;
    } else {
        delete mySetup;
        return 0;
    }
}

void SetupDialog::treeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    (void) selected;
    (void) deselected;

    _currentNetwork = 0;
    SetupDialogTreeItem *item = getSelectedItem();
    if (item) {
        switch (item->getType()) {
            case SetupDialogTreeItem::type_network:
                ui->stackedWidget->setCurrentWidget(ui->networkPage);
                _currentNetwork = item->network;
                ui->edNetworkName->setText(item->network->name());
                break;
            case SetupDialogTreeItem::type_interface_root:
                showInterfacesPage(item);
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
}

QModelIndex SetupDialog::getSelectedIndex()
{
    return ui->treeView->selectionModel()->currentIndex();
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
    CanInterfaceList list;
    if (dlg.selectInterfaces(*_backend, list, _currentNetwork->_canInterfaces)) {
        foreach (pCanInterface intf, list) {
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



void SetupDialog::showInterfacesPage(SetupDialogTreeItem *item)
{
    ui->stackedWidget->setCurrentWidget(ui->interfacesPage);
    _currentNetwork = item->network;
    ui->interfacesTreeView->setRootIndex(getSelectedIndex());
}

void SetupDialog::addCanDb(const QModelIndex &parent)
{
    QString filename = QFileDialog::getOpenFileName(this, "Load CAN Database", "", "Vector DBC Files (*.dbc)");
    if (!filename.isNull()) {
        DbcParser parser;
        QFile *dbc = new QFile(filename);
        pCanDb candb(new CanDb());
        parser.parseFile(dbc, *candb);
        delete dbc;
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
    SetupDialogTreeModel *model = static_cast<SetupDialogTreeModel *>(ui->treeView->model());
    model->deleteCanDb(getSelectedIndex());
}

void SetupDialog::on_btRemoveDatabase_clicked()
{
    SetupDialogTreeModel *model = static_cast<SetupDialogTreeModel *>(ui->treeView->model());
    model->deleteCanDb(ui->candbsTreeView->selectionModel()->currentIndex());
}

void SetupDialog::updateButtons()
{
    ui->btRemoveDatabase->setEnabled(ui->candbsTreeView->selectionModel()->hasSelection());
    ui->btRemoveInterface->setEnabled(ui->interfacesTreeView->selectionModel()->hasSelection());
}

void SetupDialog::on_btAddNetwork_clicked()
{

}

void SetupDialog::on_btRemoveNetwork_clicked()
{

}
