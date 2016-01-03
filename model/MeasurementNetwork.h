#ifndef MEASUREMENTNETWORK_H
#define MEASUREMENTNETWORK_H

#include <QString>
#include <QList>
#include <QSharedPointer>
#include <drivers/CanInterface.h>
#include <model/CanDb.h>

class MeasurementNetwork
{
public:
    MeasurementNetwork();

    void addCanInterface(QSharedPointer<CanInterface>intf);
    void addCanDb(QSharedPointer<CanDb> candb);
    QList<QSharedPointer<CanInterface> > _canInterfaces;
    QList<QSharedPointer<CanDb> > _canDbs;

    QString name() const;
    void setName(const QString &name);

private:
    QString _name;
};

#endif // MEASUREMENTNETWORK_H
