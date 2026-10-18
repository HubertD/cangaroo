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

#pragma once

#include <QThread>
#include <QVector>
#include <QMutex>
#include "CanMessage.h"
#include <driver/CanInterface.h>

class CanReplayer : public QThread
{
    Q_OBJECT
public:
    explicit CanReplayer(QObject* parent = nullptr);
    ~CanReplayer() override;

    void setMessages(const QVector<CanMessage>& msgs);
    void setTargetInterface(CanInterface* target);
    void setLooping(bool loop);
    void setPlaybackSpeed(double speed);
    void setOverrideTiming(bool override, int intervalMs);
    void setTraceBuffer(class CanTrace* trace);

    void stopReplay();
    void pauseReplay();
    void resumeReplay();

signals:
    void progressUpdated(int index, int total);
    void replayFinished();

protected:
    void run() override;

private:
    QVector<CanMessage> _messages;
    CanInterface* _target;
    class CanTrace* _traceBuffer;
    bool _looping;
    bool _running;
    bool _paused;
    double _playbackSpeed;
    bool _overrideTiming;
    int _fixedIntervalMs;
    QMutex _mutex;
};
