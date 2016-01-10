#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QList>
#include <driver/CanDriver.h>
#include <model/CanDb.h>
#include <model/MeasurementSetup.h>

class MeasurementNetwork;
class CanTrace;
class CanListener;
class CanDbMessage;


class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = 0);
    virtual ~Backend();

    void addCanDriver(CanDriver *driver);

    bool startMeasurement();
    bool stopMeasurement();
    bool isMeasurementRunning() const;
    double getMeasurementStartTime() const;



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

signals:
    void beginMeasurement();
    void endMeasurement();

public slots:

private:
    bool _measurementRunning;
    double _measurementStartTime;
    QList<CanDriver*> _drivers;
    MeasurementSetup _setup;
    CanTrace *_trace;
    QList<CanListener*> _listeners;
};

#endif // BACKEND_H
