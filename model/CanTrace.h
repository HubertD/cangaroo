#ifndef CANTRACE_H
#define CANTRACE_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QVector>

class CanInterface;
class CanMessage;
class CanDbMessage;
class MeasurementSetup;

class CanTrace : public QObject
{
    Q_OBJECT
public:
    explicit CanTrace(QObject *parent, MeasurementSetup *setup, int flushInterval);

    unsigned long size();
    void clear();

    void setSetup(MeasurementSetup *setup);

    const CanMessage *getMessage(unsigned long idx);
    CanDbMessage *findDbMessage(const CanMessage &msg);
    QString getInterfaceName(const CanInterface &interface);

    void saveCanDump(QString filename);

signals:
    void messageEnqueued(const CanMessage &msg);
    void beforeAppend(int num_messages);
    void afterAppend(int num_messages);
    void beforeClear();
    void afterClear();

public slots:
    void enqueueMessage(const CanMessage &msg, bool more_to_follow=false);
    void flushQueue();

private:
    MeasurementSetup *_setup;
    QVector<CanMessage*> _data;
    QVector<CanMessage*> _queue;

    QMutex _dataMutex;
    QMutex _queueMutex;
    QTimer _flushTimer;

};

#endif // CANTRACE_H
