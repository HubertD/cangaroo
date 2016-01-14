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

    void cloneFrom(MeasurementInterface &origin);
    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

    bool doConfigure() const;
    void setDoConfigure(bool doConfigure);

    bool isListenOnlyMode() const;
    void setListenOnlyMode(bool isListenOnlyMode);

    bool isTripleSampling() const;
    void setTripleSampling(bool isTripleSampling);

    bool isCanFD() const;
    void setCanFD(bool isCanFD);

    bool isSimpleTiming() const;
    void setSimpleTiming(bool isSimpleTiming);

    int samplePoint() const;
    void setSamplePoint(int samplePoint);

    int fdBitrate() const;
    void setFdBitrate(int fdBitrate);

    int fdSamplePoint() const;
    void setFdSamplePoint(int fdSamplePoint);

    int tq() const;
    void setTq(int tq);

    int propSeg() const;
    void setPropSeg(int propSeg);

    int phaseSeg1() const;
    void setPhaseSeg1(int phaseSeg1);

    int phaseSeg2() const;
    void setPhaseSeg2(int phaseSeg2);

    bool doSetSJW() const;
    void setDoSetSJW(bool doSetSJW);

    int SJW() const;
    void setSJW(int SJW);

    int fdTq() const;
    void setFdTq(int fdTq);

    int fdPropSeg() const;
    void setFdPropSeg(int fdPropSeg);

    int fdPhaseSeg1() const;
    void setFdPhaseSeg1(int fdPhaseSeg1);

    int fdPhaseSeg2() const;
    void setFdPhaseSeg2(int fdPhaseSeg2);

    bool doSetFdSJW() const;
    void setDoSetFdSJW(bool doSetFdSJW);

    int fdSJW() const;
    void setFdSJW(int fdSJW);

    bool doAutoRestart() const;
    void setAutoRestart(bool doAutoRestart);

    int autoRestartMs() const;
    void setAutoRestartMs(int autoRestartMs);

private:
    CanInterfaceId _canif;
    bool _doConfigure;
    bool _isListenOnlyMode;
    bool _isTripleSampling;
    bool _isCanFD;
    bool _isSimpleTiming;

    int _bitrate;
    int _samplePoint;

    int _fdBitrate;
    int _fdSamplePoint;

    int _tq;
    int _propSeg;
    int _phaseSeg1;
    int _phaseSeg2;
    bool _doSetSJW;
    int _sjw;

    int _fdTq;
    int _fdPropSeg;
    int _fdPhaseSeg1;
    int _fdPhaseSeg2;
    bool _doSetFdSJW;
    int _fdSjw;

    bool _doAutoRestart;
    int _autoRestartMs;
};

#endif // MEASUREMENTINTERFACE_H
