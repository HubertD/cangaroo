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

#include <stdint.h>
#include <QObject>
#include <QList>
#include <QMutex>
#include <QDateTime>
#include <QElapsedTimer>
#include <driver/CanDriver.h>
#include <core/CanDb.h>
#include <core/MeasurementSetup.h>
#include <core/Log.h>

class MeasurementNetwork;
class CanTrace;
class CanListener;
class CanDbMessage;
class SetupDialog;
class LogModel;

class Backend : public QObject
{
    Q_OBJECT
public:
    static Backend &instance();

    explicit Backend();
    virtual ~Backend();

    void addCanDriver(CanDriver &driver);

    bool startMeasurement();
    bool stopMeasurement();
    bool isMeasurementRunning() const;
    double getTimestampAtMeasurementStart() const;
    uint64_t getUsecsAtMeasurementStart() const;
    uint64_t getNsecsSinceMeasurementStart() const;
    uint64_t getUsecsSinceMeasurementStart() const;


    void logMessage(const QDateTime dt, const log_level_t level, const QString msg);

    MeasurementSetup &getSetup();
    void loadDefaultSetup(MeasurementSetup &setup);
    void setDefaultSetup();
    void setSetup(MeasurementSetup &new_setup);

    double currentTimeStamp() const;

    CanTrace *getTrace();
    void clearTrace();

    CanDbMessage *findDbMessage(const CanMessage &msg) const;

    CanInterfaceIdList getInterfaceList();
    CanDriver *getDriverById(CanInterfaceId id);
    CanInterface *getInterfaceById(CanInterfaceId id);
    QString getInterfaceName(CanInterfaceId id);
    QString getDriverName(CanInterfaceId id);

    CanDriver *getDriverByName(QString driverName);
    CanInterface *getInterfaceByDriverAndName(QString driverName, QString deviceName);

    pCanDb loadDbc(QString filename);

    void clearLog();
    LogModel &getLogModel() const;

signals:
    void beginMeasurement();
    void endMeasurement();

    void onSetupChanged();

    void onLogMessage(const QDateTime dt, const log_level_t level, const QString msg);

    void onSetupDialogCreated(SetupDialog &dlg);

public slots:

private:
    static Backend *_instance;

    bool _measurementRunning;
    uint64_t _measurementStartTime;
    QElapsedTimer _timerSinceStart;
    QList<CanDriver*> _drivers;
    MeasurementSetup _setup;
    CanTrace *_trace;
    QList<CanListener*> _listeners;

    LogModel *_logModel;
};
