#ifndef MEASUREMENTINTERFACE_H
#define MEASUREMENTINTERFACE_H

#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

class MeasurementInterface
{
public:
    MeasurementInterface();
    int bitrate() const;
    void setBitrate(int bitrate);

    CanInterfaceId canInterface() const;
    void setCanInterface(CanInterfaceId canif);

    void cloneFrom(MeasurementInterface *origin);

private:
    CanInterfaceId _canif;
    int _bitrate;
};

#endif // MEASUREMENTINTERFACE_H
