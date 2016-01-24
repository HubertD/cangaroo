#pragma once

#include <QWidget>

namespace Ui {
class PeakCanSetupPage;
}

class Backend;
class SetupDialog;
class MeasurementInterface;

class PeakCanSetupPage : public QWidget
{
    Q_OBJECT

public:
    explicit PeakCanSetupPage(QWidget *parent);
    ~PeakCanSetupPage();

public slots:
    void onSetupDialogCreated(SetupDialog &dlg);
    void onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi);

private slots:
    void updateUI();

private:
    Ui::PeakCanSetupPage *ui;
    MeasurementInterface *_mi;
    bool _enable_ui_updates;

    Backend &backend();
};
