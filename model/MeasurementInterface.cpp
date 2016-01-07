#include "MeasurementInterface.h"
#include <drivers/CanInterface.h>
#include <drivers/CanInterfaceProvider.h>

MeasurementInterface::MeasurementInterface()
  : _bitrate(500000)
{

}

int MeasurementInterface::bitrate() const
{
    return _bitrate;
}

void MeasurementInterface::setBitrate(int bitrate)
{
    _bitrate = bitrate;
}

pCanInterface MeasurementInterface::canInterface() const
{
    return _canif;
}

void MeasurementInterface::setCanInterface(const pCanInterface &canif)
{
    _canif = canif;
}

void MeasurementInterface::cloneFrom(MeasurementInterface *origin)
{
    _canif = origin->_canif;
    _bitrate = origin->_bitrate;
}

QString MeasurementInterface::getDriverName()
{
    if (_canif) {
        return _canif->getProvider()->getName();
    } else {
        return "";
    }
}

QString MeasurementInterface::getName()
{
    if (_canif) {
        return _canif->getName();
    } else {
        return "";
    }
}

QList<int> MeasurementInterface::getAvailableBitrates()
{
    if (_canif) {
        return _canif->getAvailableBitrates();
    } else {
        return QList<int>();
    }
}
