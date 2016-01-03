#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = 0);
    void logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

signals:
    void onLogMessage(QtMsgType type, const QString &msg);

public slots:
};

#endif // LOGGER_H
