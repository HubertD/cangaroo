#include "CanTrace.h"
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>

#include <model/CanMessage.h>
#include <driver/CanInterface.h>
#include <model/MeasurementSetup.h>

CanTrace::CanTrace(QObject *parent, MeasurementSetup *setup, int flushInterval)
  : QObject(parent),
    _setup(setup),
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

void CanTrace::setSetup(MeasurementSetup *setup)
{
    _setup = setup;
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

CanDbMessage *CanTrace::findDbMessage(const CanMessage &msg)
{
    return _setup->findDbMessage(msg);
}

QString CanTrace::getInterfaceName(const CanInterface &interface)
{
    return _setup->getInterfaceName(interface);
}

void CanTrace::saveCanDump(QString filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadWrite)) {
        QTextStream stream(&file);
        foreach (CanMessage *msg, _data) {

            QString line;
            line.append(QString().sprintf("(%.6f) ", msg->getFloatTimestamp()));
            line.append(msg->getInterface()->getName());
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

void CanTrace::enqueueMessage(const CanMessage &msg, bool more_to_follow)
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
        foreach (CanMessage *msg, _queue) {
            _data.push_back(msg);
        }
        _dataMutex.unlock();
        _queue.clear();
        emit afterAppend(num_msg);
    }

}
