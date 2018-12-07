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

#pragma once

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QVector>
#include <QMap>
#include <QFile>

#include "CanMessage.h"

class CanInterface;
class CanDbMessage;
class CanDbSignal;
class MeasurementSetup;
class Backend;

class CanTrace : public QObject
{
    Q_OBJECT

public:
    explicit CanTrace(Backend &backend, QObject *parent, int flushInterval);

    unsigned long size();
    void clear();
    const CanMessage *getMessage(int idx);
    void enqueueMessage(const CanMessage &msg, bool more_to_follow=false);

    void saveCanDump(QFile &file);
    void saveVectorAsc(QFile &file);

    bool getMuxedSignalFromCache(const CanDbSignal *signal, uint64_t *raw_value);

signals:
    void messageEnqueued(int idx);
    void beforeAppend(int num_messages);
    void afterAppend();
    void beforeClear();
    void afterClear();

private slots:
    void flushQueue();

private:
    enum {
        pool_chunk_size = 1024
    };

    Backend &_backend;

    QVector<CanMessage> _data;
    int _dataRowsUsed;
    int _newRows;
    bool _isTimerRunning;

    QMap<const CanDbSignal*,uint64_t> _muxCache;

    QMutex _mutex;
    QMutex _timerMutex;
    QTimer _flushTimer;

    void startTimer();


};
