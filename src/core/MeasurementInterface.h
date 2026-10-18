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

#pragma once

#include <QDomDocument>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>

class Backend;

class MeasurementInterface
{
public:
    MeasurementInterface();

    CanInterfaceId canInterface() const;
    void setCanInterface(CanInterfaceId canif);

    void cloneFrom(MeasurementInterface &origin);
    bool loadXML(Backend &backend, QDomElement &el);
    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

    bool doConfigure() const;
    void setDoConfigure(bool doConfigure);

    bool isListenOnlyMode() const;
    void setListenOnlyMode(bool isListenOnlyMode);

    bool isOneShotMode() const;
    void setOneShotMode(bool isOneShotMode);

    bool isTripleSampling() const;
    void setTripleSampling(bool isTripleSampling);

    bool isCanFD() const;
    void setCanFD(bool isCanFD);

    unsigned bitrate() const;
    void setBitrate(unsigned bitrate);

    int samplePoint() const;
    void setSamplePoint(int samplePoint);

    unsigned fdBitrate() const;
    void setFdBitrate(unsigned fdBitrate);

    unsigned fdSamplePoint() const;
    void setFdSamplePoint(unsigned fdSamplePoint);

    bool doAutoRestart() const;
    void setAutoRestart(bool doAutoRestart);

    int autoRestartMs() const;
    void setAutoRestartMs(int autoRestartMs);

    bool isCustomBitrate() const;
    void setCustomBitrateEn(bool customBitrate);

    bool isCustomFdBitrate() const;
    void setCustomFdBitrateEn(bool customFdBitrate);

    uint32_t customBitrate() const;
    void setCustomBitrate(uint32_t customBitrate);

    uint32_t customFdBitrate() const;
    void setCustomFdBitrate(uint32_t customFdBitrate);
private:
    CanInterfaceId _canif;

    bool _doConfigure;

    unsigned _bitrate;
    unsigned _samplePoint;

    bool _isCanFD;
    unsigned _fdBitrate;
    unsigned _fdSamplePoint;

    bool _isListenOnlyMode;
    bool _isOneShotMode;
    bool _isTripleSampling;
    bool _doAutoRestart;
    int _autoRestartMs;

    bool _isCustomBitrate;
    bool _isCustomFdBitrate;

    uint32_t _CustomBitrate;
    uint32_t _CustomFdBitrate;
};
