#include "Logger.h"

Logger::Logger(QObject *parent)
  : QObject(parent)
{

}

void Logger::logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    (void) context;
    emit onLogMessage(type, msg);
}
