#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QList>
#include <driver/CanDriver.h>

class MeasurementSetup;
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
    bool isMeasurementRunning();

    MeasurementSetup *createDefaultSetup();

    MeasurementSetup *getSetup();
    void setSetup(MeasurementSetup *setup);
    void saveCanDump(QString filename);

    CanTrace *getTrace();
    void clearTrace();

    CanDbMessage *findDbMessage(const CanMessage &msg);

    CanInterfaceIdList getInterfaceList();
    CanDriver *getDriverById(CanInterfaceId id);
    CanInterface *getInterfaceById(CanInterfaceId id);
    QString getInterfaceName(CanInterfaceId id);
    QString getDriverName(CanInterfaceId id);

signals:
    void beginMeasurement();
    void endMeasurement();

public slots:

private:
    bool _measurementRunning;
    QList<CanDriver*> _drivers;
    MeasurementSetup *_setup;
    CanTrace *_trace;
    QList<CanListener*> _listeners;
};

#endif // BACKEND_H
