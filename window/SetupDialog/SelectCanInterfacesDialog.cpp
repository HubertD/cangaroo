#include "SelectCanInterfacesDialog.h"
#include "ui_SelectCanInterfacesDialog.h"
#include <Backend.h>

SelectCanInterfacesDialog::SelectCanInterfacesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectCanInterfacesDialog)
{
    ui->setupUi(this);
    ui->treeWidget->setHeaderLabels(QStringList() << "Device" << "Driver" << "Description");
}

SelectCanInterfacesDialog::~SelectCanInterfacesDialog()
{
    delete ui;
}

bool SelectCanInterfacesDialog::selectInterfaces(Backend &backend, CanInterfaceList &selectedInterfaces, const CanInterfaceList &excludeInterfaces)
{
    ui->treeWidget->clear();

    CanInterfaceList allInterfaces;
    foreach (pCanInterface intf, backend.getInterfaceList()) {
        if (!excludeInterfaces.contains(intf)) {
            allInterfaces.append(intf);
        }
    }


    foreach (pCanInterface intf, allInterfaces) {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->treeWidget);
        treeItem->setText(0, intf->getName());
        treeItem->setText(1, intf->getProvider()->getName());
        treeItem->setText(2, "");
    }

    if (exec()==QDialog::Accepted) {
        selectedInterfaces.clear();
        foreach (QModelIndex idx, ui->treeWidget->selectionModel()->selectedRows()) {
            if (idx.isValid()) {
                selectedInterfaces.append(allInterfaces[idx.row()]);
            }
        }
        return true;
    } else {
        return false;
    }
}
