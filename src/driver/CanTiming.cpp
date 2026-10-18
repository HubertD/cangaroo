#include "CanTiming.h"

CanTiming::CanTiming(unsigned id, unsigned bitrate, unsigned bitrate_fd, unsigned samplePoint)
  : _id(id),
    _bitrate(bitrate),
    _bitrate_fd(bitrate_fd),
    _samplePoint(samplePoint)
{
}

CanTiming::CanTiming(unsigned id, unsigned bitrate, unsigned bitrate_fd, unsigned samplePoint, unsigned samplePoint_fd)
    : _id(id),
    _bitrate(bitrate),
    _bitrate_fd(bitrate_fd),
    _samplePoint(samplePoint),
    _samplePoint_fd(samplePoint_fd)
{
}


unsigned CanTiming::getId()
{
    return _id;
}

unsigned CanTiming::getBitrate()
{
    return _bitrate;
}

unsigned CanTiming::getBitrateFD()
{
    return _bitrate_fd;
}

bool CanTiming::isCanFD()
{
    return _bitrate_fd != 0;
}

unsigned CanTiming::getSamplePoint()
{
    return _samplePoint;
}

QString CanTiming::getSamplePointStr(unsigned samplePoint)
{
    return QString::number(samplePoint/10.0, 'f', 1).append("%");
}

unsigned CanTiming::getSamplePointFD()
{
    return _samplePoint_fd;
}

QString CanTiming::getSamplePointFDStr(unsigned samplePoint)
{
    return QString::number(samplePoint/10.0, 'f', 1).append("%");
}

QString CanTiming::getTitle()
{
    QString format = isCanFD() ? "%1/%3(FD) kBit/s, sample point %2" : "%1 kBit/s, sample point %2";
    return format.arg(
        QString::number(getBitrate()/1000, 'f', 2),
        getSamplePointStr(getSamplePoint()),
        QString::number(getBitrateFD()/1000, 'f', 2)
    );
}
