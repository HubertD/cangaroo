#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QList>
#include <drivers/CanInterfaceProvider.h>

class MeasurementSetup;
class MeasurementNetwork;
class CanTrace;
class CanListener;
class SocketCanInterfaceProvider;

class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend(QObject *parent = 0);
    virtual ~Backend();

    bool startMeasurement();
    bool stopMeasurement();
    bool isMeasurementRunning();

    MeasurementSetup *createDefaultSetup();

    MeasurementSetup *getSetup();
    void setSetup(MeasurementSetup *setup);
    void saveCanDump(QString filename);

    CanTrace *getTrace();

    CanInterfaceList getInterfaceList();

signals:
    void beginMeasurement();
    void endMeasurement();

public slots:

private:
    bool _measurementRunning;
    MeasurementSetup *_setup;
    CanTrace *_trace;
    QList<CanListener*> _listeners;
    SocketCanInterfaceProvider *_socketcan;
};

#endif // BACKEND_H
