/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CanTrace.h"
#include <QMutexLocker>
#include <QFile>
#include <QTextStream>

#include <core/Backend.h>
#include <core/CanMessage.h>
#include <core/CanDbMessage.h>
#include <core/CanDbSignal.h>
#include <driver/CanInterface.h>

CanTrace::CanTrace(Backend &backend, QObject *parent, int flushInterval)
  : QObject(parent),
    _backend(backend),
    _isTimerRunning(false),
    _mutex(QMutex::Recursive),
    _timerMutex(),
    _flushTimer(this)
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

const CanMessage *CanTrace::getMessage(int idx)
{
    QMutexLocker locker(&_mutex);
    if (idx >= (_dataRowsUsed + _newRows)) {
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
        startTimer();
    }

    emit messageEnqueued(idx);
}

void CanTrace::flushQueue()
{
    {
        QMutexLocker locker(&_timerMutex);
        _isTimerRunning = false;
    }

    QMutexLocker locker(&_mutex);
    if (_newRows) {
        emit beforeAppend(_newRows);

        // see if we have muxed messages. cache muxed values, if any.
        MeasurementSetup &setup = _backend.getSetup();
        for (int i=_dataRowsUsed; i<_dataRowsUsed + _newRows; i++) {
            CanMessage &msg = _data[i];
            CanDbMessage *dbmsg = setup.findDbMessage(msg);
            if (dbmsg && dbmsg->getMuxer()) {
                foreach (CanDbSignal *signal, dbmsg->getSignals()) {
                    if (signal->isMuxed() && signal->isPresentInMessage(msg)) {
                        _muxCache[signal] = signal->extractRawDataFromMessage(msg);
                    }
                }
            }
        }

        _dataRowsUsed += _newRows;
        _newRows = 0;
        emit afterAppend();
    }

}

void CanTrace::startTimer()
{
    QMutexLocker locker(&_timerMutex);
    if (!_isTimerRunning) {
        _isTimerRunning = true;
        QMetaObject::invokeMethod(&_flushTimer, "start", Qt::QueuedConnection);
    }
}

void CanTrace::saveCanDump(QFile &file)
{
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
}

void CanTrace::saveVectorAsc(QFile &file)
{
    QMutexLocker locker(&_mutex);
    QTextStream stream(&file);

    if (_data.length()<1) {
        return;
    }


    auto firstMessage = _data.first();
    double t_start = firstMessage.getFloatTimestamp();

    QLocale locale_c(QLocale::C);
    QString dt_start = locale_c.toString(firstMessage.getDateTime(), "ddd MMM dd hh:mm:ss.zzz ap yyyy");

    stream << "date " << dt_start << endl;
    stream << "base hex  timestamps absolute" << endl;
    stream << "internal events logged" << endl;
    stream << "// version 8.5.0" << endl;
    stream << "Begin Triggerblock " << dt_start << endl;
    stream << "   0.000000 Start of measurement" << endl;

    for (unsigned int i=0; i<size(); i++) {
        CanMessage &msg = _data[i];

        double t_current = msg.getFloatTimestamp();
        QString id_hex_str = QString().sprintf("%x", msg.getId());
        QString id_dec_str = QString().sprintf("%d", msg.getId());
        if (msg.isExtended()) {
            id_hex_str.append("x");
            id_dec_str.append("x");
        }

        // TODO how to handle RTR flag?
        QString line = QString().sprintf(
            "%11.6lf 1  %-15s %s   d %d %s  Length = %d BitCount = %d ID = %s",
            t_current-t_start,
            id_hex_str.toStdString().c_str(),
            "Rx", // TODO handle Rx/Tx
            msg.getLength(),
            msg.getDataHexString().toStdString().c_str(),
            0, // TODO Length (transfer time in ns)
            0, // TODO BitCount (overall frame length, including stuff bits)
            id_dec_str.toStdString().c_str()
        );

        stream << line << endl;
    }

    stream << "End TriggerBlock" << endl;
}

bool CanTrace::getMuxedSignalFromCache(const CanDbSignal *signal, uint64_t *raw_value)
{
    if (_muxCache.contains(signal)) {
        *raw_value = _muxCache[signal];
        return true;
    } else {
        return false;
    }
}
