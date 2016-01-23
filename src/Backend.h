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

#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QDateTime>
#include <driver/CanDriver.h>
#include <model/CanDb.h>
#include <model/MeasurementSetup.h>

class MeasurementNetwork;
class CanTrace;
class CanListener;
class CanDbMessage;
class SetupDialog;
class LogModel;

typedef enum log_level {
    log_level_debug,
    log_level_info,
    log_level_warning,
    log_level_error,
    log_level_critical,
    log_level_fatal
} log_level_t;

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
    double getMeasurementStartTime() const;

    void logMessage(const QDateTime dt, const log_level_t level, const QString msg);
    void logMessage(const log_level_t level, const QString msg);


    MeasurementSetup &getSetup();
    void loadDefaultSetup(MeasurementSetup &setup);
    void setDefaultSetup();
    void setSetup(MeasurementSetup &new_setup);
    void saveCanDump(QString filename);

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
    double _measurementStartTime;
    QList<CanDriver*> _drivers;
    MeasurementSetup _setup;
    CanTrace *_trace;
    QList<CanListener*> _listeners;

    LogModel *_logModel;
};

#endif // BACKEND_H
