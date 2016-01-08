#ifndef SELECTCANINTERFACESDIALOG_H
#define SELECTCANINTERFACESDIALOG_H

#include <QDialog>
#include <QList>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

class Backend;

namespace Ui {
class SelectCanInterfacesDialog;
}

class SelectCanInterfacesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectCanInterfacesDialog(QWidget *parent = 0);
    ~SelectCanInterfacesDialog();

    bool selectInterfaces(Backend &backend, CanInterfaceIdList &selectedInterfaces, const CanInterfaceIdList &excludeInterfaces);

private:
    Ui::SelectCanInterfacesDialog *ui;
};

#endif // SELECTCANINTERFACESDIALOG_H
