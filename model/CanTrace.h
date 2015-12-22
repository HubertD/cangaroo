#ifndef CANTRACE_H
#define CANTRACE_H

#include <QObject>
#include <QMutex>
#include <QTimer>

#include "CanMessage.h"
#include <vector>

class CanTrace : public QObject
{
    Q_OBJECT
public:
    explicit CanTrace(QObject *parent = 0, int flushInterval=100);

    unsigned long size();
    void clear();

    const CanMessage *getMessage(unsigned long idx);

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
    typedef std::vector<CanMessage*> CanMessageList;
    CanMessageList _data;
    CanMessageList _queue;

    QMutex _dataMutex;
    QMutex _queueMutex;
    QTimer _flushTimer;

};

#endif // CANTRACE_H
