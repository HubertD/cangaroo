#ifndef CANDLEAPISETUPPAGE_H
#define CANDLEAPISETUPPAGE_H

#include <QWidget>

namespace Ui {
class CandleApiSetupPage;
}

class CandleApiSetupPage : public QWidget
{
    Q_OBJECT

public:
    explicit CandleApiSetupPage(QWidget *parent = 0);
    ~CandleApiSetupPage();

private:
    Ui::CandleApiSetupPage *ui;
};

#endif // CANDLEAPISETUPPAGE_H
