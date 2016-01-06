#ifndef MEASUREMENTNETWORK_H
#define MEASUREMENTNETWORK_H

#include <QString>
#include <QList>
#include <drivers/CanInterface.h>
#include <model/CanDb.h>

class MeasurementNetwork
{
public:
    MeasurementNetwork();
    void cloneFrom(MeasurementNetwork &origin);

    void addCanInterface(pCanInterface intf);
    void removeCanInterface(pCanInterface intf);
    void removeCanInterface(CanInterface *intf);

    void addCanDb(pCanDb candb);
    QList<pCanInterface> _canInterfaces;
    QList<pCanDb> _canDbs;

    QString name() const;
    void setName(const QString &name);

private:
    QString _name;

    pCanInterface getSharedInterfaceForPointer(CanInterface *p);
};

#endif // MEASUREMENTNETWORK_H
