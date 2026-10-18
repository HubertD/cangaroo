/*

  Copyright (c) 2026 Jayachandran Dharuman

  This file is part of CANgaroo.

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

#include "CanReplayer.h"
#include <core/CanTrace.h>

CanReplayer::CanReplayer(QObject* parent)
    : QThread(parent)
    , _target(nullptr)
    , _traceBuffer(nullptr)
    , _looping(false)
    , _running(false)
    , _paused(false)
    , _playbackSpeed(1.0)
    , _overrideTiming(false)
    , _fixedIntervalMs(10)
{
}

CanReplayer::~CanReplayer()
{
    stopReplay();
    wait();
}

void CanReplayer::setMessages(const QVector<CanMessage>& msgs)
{
    QMutexLocker locker(&_mutex);
    _messages = msgs;
}

void CanReplayer::setTargetInterface(CanInterface* target)
{
    QMutexLocker locker(&_mutex);
    _target = target;
}

void CanReplayer::setLooping(bool loop)
{
    QMutexLocker locker(&_mutex);
    _looping = loop;
}

void CanReplayer::setPlaybackSpeed(double speed)
{
    QMutexLocker locker(&_mutex);
    _playbackSpeed = speed;
}

void CanReplayer::setOverrideTiming(bool override, int intervalMs)
{
    QMutexLocker locker(&_mutex);
    _overrideTiming = override;
    _fixedIntervalMs = intervalMs;
}

void CanReplayer::setTraceBuffer(CanTrace* trace)
{
    QMutexLocker locker(&_mutex);
    _traceBuffer = trace;
}

void CanReplayer::stopReplay()
{
    QMutexLocker locker(&_mutex);
    _running = false;
    _paused = false;
}

void CanReplayer::pauseReplay()
{
    QMutexLocker locker(&_mutex);
    _paused = true;
}

void CanReplayer::resumeReplay()
{
    QMutexLocker locker(&_mutex);
    _paused = false;
}

void CanReplayer::run()
{
    {
        QMutexLocker locker(&_mutex);
        _running = true;
        _paused = false;
    }

    while (true) {
        QVector<CanMessage> msgs;
        CanInterface* target = nullptr;
        CanTrace* traceBuffer = nullptr;

        {
            QMutexLocker locker(&_mutex);
            if (!_running || _messages.isEmpty() || !_target) {
                break;
            }
            msgs = _messages;
            target = _target;
            traceBuffer = _traceBuffer;
        }

        double previous_timestamp = 0;
        if (!msgs.isEmpty()) {
            previous_timestamp = msgs.first().getFloatTimestamp();
        }

        for (int i = 0; i < msgs.size(); ++i) {
            {
                QMutexLocker locker(&_mutex);
                if (!_running) break;
            }

            // Pause handling
            while (true) {
                bool is_paused = false;
                {
                    QMutexLocker locker(&_mutex);
                    is_paused = _paused;
                    if (!_running) break;
                }
                if (!is_paused) break;
                QThread::msleep(50);
            }

            {
                QMutexLocker locker(&_mutex);
                if (!_running) break;
            }

            const CanMessage& msg = msgs[i];
            double current_timestamp = msg.getFloatTimestamp();
            
            // Wait for proper timing interval
            double diff = current_timestamp - previous_timestamp;
            
            double playbackSpeed = 1.0;
            bool overrideTiming = false;
            int fixedInterval = 10;
            
            {
                QMutexLocker locker(&_mutex);
                playbackSpeed = _playbackSpeed;
                overrideTiming = _overrideTiming;
                fixedInterval = _fixedIntervalMs;
            }

            if (overrideTiming) {
                QThread::msleep(fixedInterval);
            } else if (diff > 0) {
                if (playbackSpeed <= 0.001) playbackSpeed = 0.001; // Avoid division by zero
                double scaledDiff = diff / playbackSpeed;
                QThread::usleep(static_cast<unsigned long>(scaledDiff * 1000000.0));
            }
            previous_timestamp = current_timestamp;

            if (target) {
                CanMessage replayMsg = msg;
                replayMsg.setRX(false); // Mark as TX
                
                target->sendMessage(replayMsg);
                
                if (traceBuffer) {
                    replayMsg.setInterfaceId(target->getId());
                    traceBuffer->enqueueMessage(replayMsg);
                }
            }

            // Update UI periodically
            if (i % 10 == 0 || i == msgs.size() - 1) {
                emit progressUpdated(i + 1, msgs.size());
            }
        }

        {
            QMutexLocker locker(&_mutex);
            if (!_running || !_looping) {
                break;
            }
        }
    }

    {
        QMutexLocker locker(&_mutex);
        _running = false;
    }

    emit replayFinished();
}
