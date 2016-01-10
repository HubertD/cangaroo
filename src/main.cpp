#include "mainwindow.h"
#include <QApplication>
#include "Logger.h"

Logger logger;
void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    logger.logMessage(type, context, msg);
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);
    QApplication a(argc, argv);
    MainWindow w(&logger);
    w.show();

    return a.exec();
}
