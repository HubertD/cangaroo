#ifndef CANDLEAPISETUPPAGE_H
#define CANDLEAPISETUPPAGE_H

#include <QWidget>

namespace Ui {
class CandleApiSetupPage;
}

class Backend;
class SetupDialog;
class MeasurementInterface;

class CandleApiSetupPage : public QWidget
{
    Q_OBJECT

public:
    explicit CandleApiSetupPage(QWidget *parent = 0);
    ~CandleApiSetupPage();

public slots:
    void onSetupDialogCreated(SetupDialog &dlg);
    void onShowInterfacePage(SetupDialog &dlg, MeasurementInterface *mi);

private:
    Ui::CandleApiSetupPage *ui;
    MeasurementInterface *_mi;

    Backend &backend();
};

#endif // CANDLEAPISETUPPAGE_H
