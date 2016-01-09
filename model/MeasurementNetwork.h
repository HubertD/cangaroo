#ifndef MEASUREMENTNETWORK_H
#define MEASUREMENTNETWORK_H

#include <QString>
#include <QList>
#include <QDomDocument>

#include <driver/CanDriver.h>
#include <driver/CanInterface.h>
#include <model/CanDb.h>

class Backend;
class MeasurementInterface;

class MeasurementNetwork
{
public:
    MeasurementNetwork();
    void cloneFrom(MeasurementNetwork &origin);

    void addInterface(MeasurementInterface *intf);
    void removeInterface(MeasurementInterface *intf);
    QList<MeasurementInterface*> interfaces();

    MeasurementInterface *addCanInterface(CanInterfaceId canif);
    CanInterfaceIdList getReferencedCanInterfaces();

    void addCanDb(pCanDb candb);
    QList<pCanDb> _canDbs;

    QString name() const;
    void setName(const QString &name);

    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    bool loadXML(Backend &backend, QDomElement el);

private:
    QString _name;
    QList<MeasurementInterface*> _interfaces;
};

#endif // MEASUREMENTNETWORK_H
