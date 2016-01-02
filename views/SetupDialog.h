#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>

class MeasurementSetup;

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

    void showSetupDialog(MeasurementSetup *setup);

private:
    Ui::SetupDialog *ui;
};

#endif // SETUPDIALOG_H
