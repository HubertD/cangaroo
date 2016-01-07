#ifndef MEASUREMENTINTERFACE_H
#define MEASUREMENTINTERFACE_H

#include <driver/CanInterface.h>

class MeasurementInterface
{
public:
    MeasurementInterface();
    int bitrate() const;
    void setBitrate(int bitrate);

    pCanInterface canInterface() const;
    void setCanInterface(const pCanInterface &canif);

    void cloneFrom(MeasurementInterface *origin);

    QString getDriverName();
    QString getName();
    QList<int> getAvailableBitrates();

private:
    pCanInterface _canif;
    int _bitrate;
};

#endif // MEASUREMENTINTERFACE_H
