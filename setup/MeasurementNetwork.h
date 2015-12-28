#ifndef MEASUREMENTNETWORK_H
#define MEASUREMENTNETWORK_H

#include <QList>
#include <drivers/CanInterface.h>
#include <model/CanDb.h>

class MeasurementNetwork
{
public:
    MeasurementNetwork();

    void addCanInterface(CanInterface *intf);
    void addCanDb(CanDb *candb);
    QList<CanInterface*> _canInterfaces;
    QList<CanDb*> _canDbs;

};

#endif // MEASUREMENTNETWORK_H
