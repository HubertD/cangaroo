#ifndef CANTRACE_H
#define CANTRACE_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QVector>

#include "CanMessage.h"

class CanInterface;
class CanDbMessage;
class MeasurementSetup;

class CanTrace : public QObject
{
    Q_OBJECT

public:
    explicit CanTrace(QObject *parent, int flushInterval);

    unsigned long size();
    void clear();
    const CanMessage *getMessage(unsigned long idx);

    void saveCanDump(QString filename);

signals:
    void messageEnqueued(const CanMessage &msg);
    void beforeAppend(int num_messages);
    void afterAppend();
    void beforeClear();
    void afterClear();

public slots:
    void enqueueMessage(const CanMessage &msg, bool more_to_follow=false);

private slots:
    void flushQueue();

private:
    enum {
        pool_chunk_size = 1024
    };

    QVector<CanMessage> _data;
    int _dataRowsUsed;
    int _newRows;

    QMutex _mutex;
    QTimer _flushTimer;

};

#endif // CANTRACE_H
