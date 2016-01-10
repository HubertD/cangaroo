#include "LogWindow.h"
#include "ui_LogWindow.h"

#include <QDomDocument>

#include "Logger.h"


LogWindow::LogWindow(QWidget *parent, Logger &logger) :
    MdiWindow(parent),
    ui(new Ui::LogWindow)
{
    ui->setupUi(this);
    connect(&logger, SIGNAL(onLogMessage(QtMsgType,QString)), this, SLOT(onLogMessage(QtMsgType,QString)));
}

LogWindow::~LogWindow()
{
    delete ui;
}

bool LogWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!MdiWindow::saveXML(backend, xml, root)) { return false; }
    root.setAttribute("type", "LogWindow");
    return true;
}

bool LogWindow::loadXML(Backend &backend, QDomElement &el)
{
    return MdiWindow::loadXML(backend, el);
}

void LogWindow::onLogMessage(QtMsgType type, const QString &msg)
{
    (void) type;
    ui->loglist->addItem(msg);
    ui->loglist->scrollToBottom();
}
