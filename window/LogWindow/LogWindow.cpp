#include "LogWindow.h"
#include "ui_LogWindow.h"
#include "Logger.h"

LogWindow::LogWindow(QWidget *parent, Logger *logger) :
    QWidget(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);
    connect(logger, SIGNAL(onLogMessage(QtMsgType,QString)), this, SLOT(onLogMessage(QtMsgType,QString)));
}

LogWindow::~LogWindow()
{
    delete ui;
}

void LogWindow::onLogMessage(QtMsgType type, const QString &msg)
{
    (void) type;
    ui->loglist->addItem(msg);
    ui->loglist->scrollToBottom();
}
