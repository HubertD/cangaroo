#include "CanTrace.h"
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>

#include <Backend.h>
#include <model/CanMessage.h>
#include <driver/CanInterface.h>

CanTrace::CanTrace(Backend &backend, QObject *parent, int flushInterval)
  : QObject(parent),
    _backend(backend),
    _mutex(QMutex::Recursive)
{
    clear();
    _flushTimer.setSingleShot(true);
    _flushTimer.setInterval(flushInterval);
    connect(&_flushTimer, SIGNAL(timeout()), this, SLOT(flushQueue()));
}

unsigned long CanTrace::size()
{
    QMutexLocker locker(&_mutex);
    return _dataRowsUsed;
}

void CanTrace::clear()
{
    QMutexLocker locker(&_mutex);
    emit beforeClear();
    _data.resize(pool_chunk_size);
    _dataRowsUsed = 0;
    _newRows = 0;
    emit afterClear();
}

const CanMessage *CanTrace::getMessage(unsigned long idx)
{
    QMutexLocker locker(&_mutex);
    if (idx >= size()) {
        return 0;
    } else {
        return &_data[idx];
    }
}

void CanTrace::enqueueMessage(const CanMessage &msg, bool more_to_follow)
{
    QMutexLocker locker(&_mutex);

    int idx = size() + _newRows;
    if (idx>=_data.size()) {
        _data.resize(_data.size() + pool_chunk_size);
    }

    _data[idx].cloneFrom(msg);
    _newRows++;

    if (!more_to_follow) {
        if (!_flushTimer.isActive()) {
            _flushTimer.start();
        }
    }

    emit messageEnqueued(_data[idx]);
}

void CanTrace::flushQueue()
{
    QMutexLocker locker(&_mutex);
    if (_newRows) {
        emit beforeAppend(_newRows);
        _dataRowsUsed += _newRows;
        _newRows = 0;
        emit afterAppend();
    }

}

void CanTrace::saveCanDump(QString filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QMutexLocker locker(&_mutex);
        QTextStream stream(&file);
        for (unsigned int i=0; i<size(); i++) {
            CanMessage *msg = &_data[i];
            QString line;
            line.append(QString().sprintf("(%.6f) ", msg->getFloatTimestamp()));
            line.append(_backend.getInterfaceName(msg->getInterfaceId()));
            if (msg->isExtended()) {
                line.append(QString().sprintf(" %08X#", msg->getId()));
            } else {
                line.append(QString().sprintf(" %03X#", msg->getId()));
            }
            for (int i=0; i<msg->getLength(); i++) {
                line.append(QString().sprintf("%02X", msg->getByte(i)));
            }
            stream << line << endl;
        }
        file.close();
    }
}
