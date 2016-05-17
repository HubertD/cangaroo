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

#include <core/Backend.h>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

MeasurementInterface::MeasurementInterface()
  : _doConfigure(false),
    _isListenOnlyMode(false),
    _isOneShotMode(false),
    _isTripleSampling(false),
    _isCanFD(false),
    _isSimpleTiming(true),
    _bitrate(500000),
    _samplePoint(875),
    _fdBitrate(4000000),
    _fdSamplePoint(875),
    _tq(125),
    _propSeg(6),
    _phaseSeg1(7),
    _phaseSeg2(2),
    _doSetSJW(false),
    _sjw(1),
    _fdTq(12),
    _fdPropSeg(7),
    _fdPhaseSeg1(2),
    _fdPhaseSeg2(1),
    _doSetFdSJW(false),
    _fdSjw(1),
    _doAutoRestart(false),
    _autoRestartMs(100)
{

}

bool MeasurementInterface::loadXML(Backend &backend, QDomElement &el)
{
    (void) backend;

    _doConfigure = el.attribute("configure", "0").toInt() != 0;
    _isListenOnlyMode = el.attribute("listen-only", "0").toInt() != 0;
    _isOneShotMode = el.attribute("one-shot", "0").toInt() != 0;
    _isTripleSampling = el.attribute("triple-sampling", "0").toInt() != 0;
    _isCanFD = el.attribute("can-fd", "0").toInt() != 0;
    _isSimpleTiming = el.attribute("timing-mode", "bitrate") == "bitrate";
    _doAutoRestart = el.attribute("auto-restart", "0").toInt() != 0;
    _autoRestartMs = el.attribute("auto-restart-time", "100").toInt();

    QDomElement simpleTimingNode = el.firstChildElement("timing-bitrate");
    _bitrate = simpleTimingNode.attribute("bitrate", "500000").toInt();
    _samplePoint = simpleTimingNode.attribute("sample-point", "0").toInt();
    _fdBitrate = simpleTimingNode.attribute("bitrate-fd", "500000").toInt();
    _fdSamplePoint = simpleTimingNode.attribute("sample-point-fd", "0").toInt();

    QDomElement advTimingNode = el.firstChildElement("timing-registers");

    QDomElement advTimingCan = advTimingNode.firstChildElement("can");
    _tq = advTimingCan.attribute("tq", "0").toInt();
    _propSeg = advTimingCan.attribute("prop-seg", "0").toInt();
    _phaseSeg1 = advTimingCan.attribute("phase-seg1", "0").toInt();
    _phaseSeg2 = advTimingCan.attribute("phase-seg2", "0").toInt();
    _doSetSJW = advTimingCan.attribute("set-sjw", "0").toInt() != 0;
    _sjw = advTimingCan.attribute("sjw", "1").toInt();

    QDomElement advTimingCanFd = advTimingNode.firstChildElement("can-fd");
    _fdTq = advTimingCanFd.attribute("tq", "0").toInt();
    _fdPropSeg = advTimingCanFd.attribute("prop-seg", "0").toInt();
    _fdPhaseSeg1 = advTimingCanFd.attribute("phase-seg1", "0").toInt();
    _fdPhaseSeg2 = advTimingCanFd.attribute("phase-seg2", "0").toInt();
    _doSetFdSJW = advTimingCanFd.attribute("set-sjw", "0").toInt() != 0;
    _fdSjw = advTimingCanFd.attribute("sjw", "1").toInt();

    return true;
}

bool MeasurementInterface::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    (void) xml;
    root.setAttribute("type", "can");
    root.setAttribute("driver", backend.getDriverName(_canif));
    root.setAttribute("name", backend.getInterfaceName(_canif));

    root.setAttribute("configure", _doConfigure ? 1 : 0);
    root.setAttribute("listen-only", _isListenOnlyMode ? 1 : 0);
    root.setAttribute("one-shot", _isOneShotMode ? 1 : 0);
    root.setAttribute("triple-sampling", _isTripleSampling ? 1 : 0);
    root.setAttribute("can-fd", _isCanFD ? 1 : 0);
    root.setAttribute("timing-mode", _isSimpleTiming ? "bitrate" : "registers");
    root.setAttribute("auto-restart", _doAutoRestart ? 1 : 0);
    root.setAttribute("auto-restart-time", _autoRestartMs);

    QDomElement simpleTimingNode = xml.createElement("timing-bitrate");
    simpleTimingNode.setAttribute("bitrate", _bitrate);
    simpleTimingNode.setAttribute("sample-point", _samplePoint);
    simpleTimingNode.setAttribute("bitrate-fd", _fdBitrate);
    simpleTimingNode.setAttribute("sample-point-fd", _fdSamplePoint);
    root.appendChild(simpleTimingNode);

    QDomElement advTimingNode = xml.createElement("timing-registers");
    QDomElement advTimingCan = xml.createElement("can");
    advTimingCan.setAttribute("tq", _tq);
    advTimingCan.setAttribute("prop-seg", _propSeg);
    advTimingCan.setAttribute("phase-seg1", _phaseSeg1);
    advTimingCan.setAttribute("phase-seg2", _phaseSeg2);
    advTimingCan.setAttribute("set-sjw", _doSetSJW ? 1 : 0);
    advTimingCan.setAttribute("sjw", _sjw);
    advTimingNode.appendChild(advTimingCan);

    QDomElement advTimingCanFd = xml.createElement("can-fd");
    advTimingCanFd.setAttribute("tq", _fdTq);
    advTimingCanFd.setAttribute("prop-seg", _fdPropSeg);
    advTimingCanFd.setAttribute("phase-seg1", _fdPhaseSeg1);
    advTimingCanFd.setAttribute("phase-seg2", _fdPhaseSeg2);
    advTimingCanFd.setAttribute("set-sjw", _doSetFdSJW ? 1 : 0);
    advTimingCanFd.setAttribute("sjw", _fdSjw);
    advTimingNode.appendChild(advTimingCanFd);

    root.appendChild(advTimingNode);

    return true;
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

bool MeasurementInterface::isOneShotMode() const
{
    return _isOneShotMode;
}

void MeasurementInterface::setOneShotMode(bool isOneShotMode)
{
    _isOneShotMode = isOneShotMode;
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
