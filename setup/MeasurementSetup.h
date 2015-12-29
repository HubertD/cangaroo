#ifndef MEASUREMENTSETUP_H
#define MEASUREMENTSETUP_H

#include "MeasurementSetup.h"

#include <QObject>
#include <QList>
#include <model/CanTrace.h>
#include <model/CanDbMessage.h>

#include "MeasurementNetwork.h"

class MeasurementSetup : public QObject
{
    //Q_OBJECT

public:
    MeasurementSetup();
    ~MeasurementSetup();
    MeasurementNetwork *createNetwork();

    void startMeasurement();
    void stopMeasurement();

    CanTrace *getTrace();
    CanDbMessage *findDbMessage(const CanMessage *msg);
    QString getInterfaceName(CanInterface *interface);
private:
    CanTrace _trace;
    QList<MeasurementNetwork*> _networks;
};

#endif // MEASUREMENTSETUP_H
