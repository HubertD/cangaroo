/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "MeasurementInterface.h"

#include <Backend.h>
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

void MeasurementInterface::cloneFrom(MeasurementInterface &origin)
{
    *this = origin;
}

bool MeasurementInterface::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    (void) xml;
    root.setAttribute("type", "can");
    root.setAttribute("driver", backend.getDriverName(_canif));
    root.setAttribute("name", backend.getInterfaceName(_canif));
    root.setAttribute("bitrate", _bitrate);
    return true;
}
bool MeasurementInterface::doConfigure() const
{
    return _doConfigure;
}

void MeasurementInterface::setDoConfigure(bool doConfigure)
{
    _doConfigure = doConfigure;
}
bool MeasurementInterface::isListenOnlyMode() const
{
    return _isListenOnlyMode;
}

void MeasurementInterface::setListenOnlyMode(bool isListenOnlyMode)
{
    _isListenOnlyMode = isListenOnlyMode;
}
bool MeasurementInterface::isTripleSampling() const
{
    return _isTripleSampling;
}

void MeasurementInterface::setTripleSampling(bool isTripleSampling)
{
    _isTripleSampling = isTripleSampling;
}
bool MeasurementInterface::isCanFD() const
{
    return _isCanFD;
}

void MeasurementInterface::setCanFD(bool isCanFD)
{
    _isCanFD = isCanFD;
}
bool MeasurementInterface::isSimpleTiming() const
{
    return _isSimpleTiming;
}

void MeasurementInterface::setSimpleTiming(bool isSimpleTiming)
{
    _isSimpleTiming = isSimpleTiming;
}
int MeasurementInterface::samplePoint() const
{
    return _samplePoint;
}

void MeasurementInterface::setSamplePoint(int samplePoint)
{
    _samplePoint = samplePoint;
}
int MeasurementInterface::fdBitrate() const
{
    return _fdBitrate;
}

void MeasurementInterface::setFdBitrate(int fdBitrate)
{
    _fdBitrate = fdBitrate;
}
int MeasurementInterface::fdSamplePoint() const
{
    return _fdSamplePoint;
}

void MeasurementInterface::setFdSamplePoint(int fdSamplePoint)
{
    _fdSamplePoint = fdSamplePoint;
}
int MeasurementInterface::tq() const
{
    return _tq;
}

void MeasurementInterface::setTq(int tq)
{
    _tq = tq;
}
int MeasurementInterface::propSeg() const
{
    return _propSeg;
}

void MeasurementInterface::setPropSeg(int propSeg)
{
    _propSeg = propSeg;
}
int MeasurementInterface::phaseSeg1() const
{
    return _phaseSeg1;
}

void MeasurementInterface::setPhaseSeg1(int phaseSeg1)
{
    _phaseSeg1 = phaseSeg1;
}
int MeasurementInterface::phaseSeg2() const
{
    return _phaseSeg2;
}

void MeasurementInterface::setPhaseSeg2(int phaseSeg2)
{
    _phaseSeg2 = phaseSeg2;
}
bool MeasurementInterface::doSetSJW() const
{
    return _doSetSJW;
}

void MeasurementInterface::setDoSetSJW(bool doSetSJW)
{
    _doSetSJW = doSetSJW;
}
int MeasurementInterface::SJW() const
{
    return _sjw;
}

void MeasurementInterface::setSJW(int sjw)
{
    _sjw = sjw;
}
int MeasurementInterface::fdTq() const
{
    return _fdTq;
}

void MeasurementInterface::setFdTq(int fdTq)
{
    _fdTq = fdTq;
}
int MeasurementInterface::fdPropSeg() const
{
    return _fdPropSeg;
}

void MeasurementInterface::setFdPropSeg(int fdPropSeg)
{
    _fdPropSeg = fdPropSeg;
}
int MeasurementInterface::fdPhaseSeg1() const
{
    return _fdPhaseSeg1;
}

void MeasurementInterface::setFdPhaseSeg1(int fdPhaseSeg1)
{
    _fdPhaseSeg1 = fdPhaseSeg1;
}
int MeasurementInterface::fdPhaseSeg2() const
{
    return _fdPhaseSeg2;
}

void MeasurementInterface::setFdPhaseSeg2(int fdPhaseSeg2)
{
    _fdPhaseSeg2 = fdPhaseSeg2;
}
bool MeasurementInterface::doSetFdSJW() const
{
    return _doSetFdSJW;
}

void MeasurementInterface::setDoSetFdSJW(bool doSetFdSJW)
{
    _doSetFdSJW = doSetFdSJW;
}
int MeasurementInterface::fdSJW() const
{
    return _fdSjw;
}

void MeasurementInterface::setFdSJW(int fdSjw)
{
    _fdSjw = fdSjw;
}
bool MeasurementInterface::doAutoRestart() const
{
    return _doAutoRestart;
}

void MeasurementInterface::setAutoRestart(bool doAutoRestart)
{
    _doAutoRestart = doAutoRestart;
}
int MeasurementInterface::autoRestartMs() const
{
    return _autoRestartMs;
}

void MeasurementInterface::setAutoRestartMs(int autoRestartMs)
{
    _autoRestartMs = autoRestartMs;
}























