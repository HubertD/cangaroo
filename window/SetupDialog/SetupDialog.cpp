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

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog),
    tree_model(0),
    _currentNetwork(0)
{
    ui->setupUi(this);
    QIcon icon(":/assets/icon.png");
    setWindowIcon(icon);

    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenu(QPoint)));

    _actionDeleteInterface = new QAction("Delete", this);
    _actionDeleteCanDb = new QAction("Delete", this);
    _actionAddInterface = new QAction("Add...", this);
    _actionAddCanDb = new QAction("Add...", this);
    connect(_actionAddCanDb, SIGNAL(triggered()), this, SLOT(addCanDb()));
    connect(_actionDeleteCanDb, SIGNAL(triggered()), this, SLOT(deleteCanDb()));

    connect(ui->edNetworkName, SIGNAL(textChanged(QString)), this, SLOT(edNetworkNameChanged()));
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

MeasurementSetup *SetupDialog::showSetupDialog(MeasurementSetup &setup)
{
    MeasurementSetup *mySetup = new MeasurementSetup(0);
    mySetup->cloneFrom(setup);

    if (tree_model) {
        delete tree_model;
    }
    tree_model = new SetupDialogTreeModel(mySetup, this);

    ui->treeView->setModel(tree_model);
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(treeViewSelectionChanged(QItemSelection,QItemSelection)));
    ui->treeView->expandAll();

    if (exec()==QDialog::Accepted) {
        return mySetup;
    } else {
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
            default:
                ui->stackedWidget->setCurrentWidget(ui->emptyPage);
                break;

        }
    }
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
        tree_model->dataChanged(getSelectedIndex(), getSelectedIndex());
    }
}

void SetupDialog::addCanDb()
{
    SetupDialogTreeItem *parentItem = getSelectedItem();
    if (!parentItem) { return; }

    SetupDialogTreeModel *model = static_cast<SetupDialogTreeModel *>(ui->treeView->model());

    QString filename = QFileDialog::getOpenFileName(this, "Load CAN Database", "", "Vector DBC Files (*.dbc)");
    if (!filename.isNull()) {
        DbcParser parser;
        QFile *dbc = new QFile(filename);
        pCanDb candb(new CanDb());
        parser.parseFile(dbc, *candb);
        delete dbc;
        model->addCanDb(getSelectedIndex(), candb);
    }
}

void SetupDialog::deleteCanDb()
{
    SetupDialogTreeModel *model = static_cast<SetupDialogTreeModel *>(ui->treeView->model());
    model->deleteCanDb(getSelectedIndex());
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

void SetupDialog::showInterfacesPage(SetupDialogTreeItem *item)
{
    ui->stackedWidget->setCurrentWidget(ui->interfacesPage);
    _currentNetwork = item->network;
    QTreeWidget *tree = ui->interfacesTreeWidget;

    tree->clear();
    foreach (pCanInterface intf, item->network->_canInterfaces) {
        QTreeWidgetItem *ti = new QTreeWidgetItem(tree);
        ti->setText(0, intf->getName());
        ti->setText(1, intf->getProvider()->getName());
        ti->setData(0, Qt::UserRole, QVariant::fromValue((void*)intf.data()));
    }

}

void SetupDialog::addInterface()
{

}

void SetupDialog::on_btAddInterface_clicked()
{
    // todo add Interface
}


void SetupDialog::on_btRemoveInterface_clicked()
{
    QTreeWidget *tree = ui->interfacesTreeWidget;
    QTreeWidgetItem *item = tree->currentItem();
    if (item) {
        void *p = item->data(0, Qt::UserRole).value<void*>();
        CanInterface *intf = static_cast<CanInterface*>(p);
        _currentNetwork->removeCanInterface(intf);
        delete item;
    }
}
