#include "CanTrace.h"
#include <QMutexLocker>

CanTrace::CanTrace(QObject *parent, int flushInterval)
  : QObject(parent),
    _dataMutex(QMutex::Recursive),
    _queueMutex(QMutex::Recursive)
{
    _flushTimer.setSingleShot(true);
    _flushTimer.setInterval(flushInterval);
    connect(&_flushTimer, SIGNAL(timeout()), this, SLOT(flushQueue()));
}

unsigned long CanTrace::size()
{
    QMutexLocker dataLocker(&_dataMutex);
    return _data.size();
}

void CanTrace::clear()
{
    QMutexLocker dataLocker(&_dataMutex);
    QMutexLocker queueLocker(&_queueMutex);
    emit beforeClear();
    _data.clear();
    _queue.clear();
    emit afterClear();
}

const CanMessage *CanTrace::getMessage(unsigned long idx)
{
    QMutexLocker dataLocker(&_dataMutex);
    if (idx >= _data.size()) {
        return 0;
    } else {
        return _data[idx];
    }
}

void CanTrace::enqueueMessage(int interface_id, const CanMessage &msg, bool more_to_follow)
{
    _queueMutex.lock();
    _queue.push_back(new CanMessage(msg));
    _queueMutex.unlock();

    if (!more_to_follow) {
        if (!_flushTimer.isActive()) {
            _flushTimer.start();
        }
    }

    emit messageEnqueued(msg);
}

void CanTrace::flushQueue()
{
    QMutexLocker queueLocker(&_queueMutex);

    unsigned long num_msg = _queue.size();
    if (num_msg) {
        emit beforeAppend(num_msg);
        _dataMutex.lock();
        for (CanMessageList::iterator it=_queue.begin(); it!=_queue.end(); ++it) {
            _data.push_back(*it);
        }
        _dataMutex.unlock();
        _queue.clear();
        emit afterAppend(num_msg);
    }

}
