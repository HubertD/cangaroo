#include "SetupDialog.h"
#include "ui_SetupDialog.h"

#include <model/MeasurementSetup.h>
#include "SetupDialogTreeModel.h"
#include "SetupDialogTreeItem.h"
#include <QItemSelectionModel>
#include <QMenu>
#include <QFileDialog>
#include <parser/dbc/DbcParser.h>

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog)
{
    ui->setupUi(this);
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(treeViewContextMenu(QPoint)));

    _actionDeleteInterface = new QAction("Delete", this);
    _actionDeleteCanDb = new QAction("Delete", this);
    _actionAddInterface = new QAction("Add...", this);
    _actionAddCanDb = new QAction("Add...", this);
    connect(_actionAddCanDb, SIGNAL(triggered()), this, SLOT(addCanDb()));
    connect(_actionDeleteCanDb, SIGNAL(triggered()), this, SLOT(deleteCanDb()));

}

SetupDialog::~SetupDialog()
{
    delete ui;
}

void SetupDialog::showSetupDialog(MeasurementSetup *setup)
{
    SetupDialogTreeModel model(setup, this);
    ui->treeView->setModel(&model);
    connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(treeViewSelectionChanged(QItemSelection,QItemSelection)));
    ui->treeView->expandAll();
    if (exec()==QDialog::Accepted) {
        // TODO detect changes, update MeasurementSetup
    }
}

void SetupDialog::treeViewSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    (void) selected;
    (void) deselected;

    SetupDialogTreeItem *item = getSelectedItem();
    if (item) {
        switch (item->getType()) {
            case SetupDialogTreeItem::type_network:
                ui->stackedWidget->setCurrentWidget(ui->networkPage);
                ui->lineEdit->setText(item->network->name());
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
    QAction* selectedItem = contextMenu.exec(globalPos);
    if (selectedItem)
    {
    }
    else
    {
    }
}

void SetupDialog::addInterface()
{

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
        CanDb *candb = new CanDb();
        parser.parseFile(dbc, candb);
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
