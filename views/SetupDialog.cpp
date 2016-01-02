#include "SetupDialog.h"
#include "ui_SetupDialog.h"

#include <setup/MeasurementSetup.h>
#include "SetupDialogTreeModel.h"

SetupDialog::SetupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SetupDialog)
{
    ui->setupUi(this);
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

void SetupDialog::showSetupDialog(MeasurementSetup *setup)
{
    SetupDialogTreeModel model(setup, this);
    ui->treeView->setModel(&model);
    ui->treeView->expandAll();
    exec();
}
