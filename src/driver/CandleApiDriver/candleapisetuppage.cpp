#include "CandleApiSetupPage.h"
#include "ui_candleapisetuppage.h"

CandleApiSetupPage::CandleApiSetupPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CandleApiSetupPage)
{
    ui->setupUi(this);
}

CandleApiSetupPage::~CandleApiSetupPage()
{
    delete ui;
}
