#include "LogView.h"
#include "ui_LogView.h"
#include "Logger.h"

LogView::LogView(QWidget *parent, Logger *logger) :
    QWidget(parent),
    ui(new Ui::LogView)
{
    ui->setupUi(this);
    connect(logger, SIGNAL(onLogMessage(QtMsgType,QString)), this, SLOT(onLogMessage(QtMsgType,QString)));
}

LogView::~LogView()
{
    delete ui;
}

void LogView::onLogMessage(QtMsgType type, const QString &msg)
{
    ui->loglist->addItem(msg);
}
