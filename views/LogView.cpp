#include "LogView.h"
#include "ui_LogView.h"

LogView::LogView(QWidget *parent, MeasurementSetup *setup) :
    QWidget(parent),
    ui(new Ui::LogView),
    _setup(setup)
{
    ui->setupUi(this);

//    connect(setup, SIGNAL(appendLog(log_level_t, QString)), this, SLOT(appendLog(log_level_t,QString)));
}

LogView::~LogView()
{
    delete ui;
}
/*
void LogView::appendLog(log_level_t level, QString s)
{
    ui->loglist->addItem(s);
}
*/
