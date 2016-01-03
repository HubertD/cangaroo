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

    void addCanInterface(CanInterface *intf);
    void addCanDb(CanDb *candb);
    QList<CanInterface*> _canInterfaces;
    QList<CanDb*> _canDbs;

    QString name() const;
    void setName(const QString &name);

private:
    QString _name;
};

#endif // MEASUREMENTNETWORK_H
