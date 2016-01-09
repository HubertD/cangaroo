#ifndef MEASUREMENTINTERFACE_H
#define MEASUREMENTINTERFACE_H

#include <QDomDocument>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

class Backend;

class MeasurementInterface
{
public:
    MeasurementInterface();
    int bitrate() const;
    void setBitrate(int bitrate);

    CanInterfaceId canInterface() const;
    void setCanInterface(CanInterfaceId canif);

    void cloneFrom(MeasurementInterface *origin);
    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

private:
    CanInterfaceId _canif;
    int _bitrate;
};

#endif // MEASUREMENTINTERFACE_H
