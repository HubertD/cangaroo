#include "MeasurementInterface.h"
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

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

CanInterfaceId MeasurementInterface::canInterface() const
{
    return _canif;
}

void MeasurementInterface::setCanInterface(CanInterfaceId canif)
{
    _canif = canif;
}

void MeasurementInterface::cloneFrom(MeasurementInterface *origin)
{
    _canif = origin->_canif;
    _bitrate = origin->_bitrate;
}

