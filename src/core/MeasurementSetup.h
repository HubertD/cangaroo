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

#include <QObject>
#include <QList>
#include <QDomDocument>

class Backend;
class MeasurementNetwork;
class CanTrace;
class CanMessage;
class CanInterface;
class CanDbMessage;

class MeasurementSetup : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementSetup(QObject *parent);
    virtual ~MeasurementSetup();
    void clear();

    CanDbMessage *findDbMessage(const CanMessage &msg) const;
    QString getInterfaceName(const CanInterface &interface) const;

    int countNetworks() const;
    MeasurementNetwork *getNetwork(int index) const;
    MeasurementNetwork *getNetworkByName(QString name) const;
    QList<MeasurementNetwork*> getNetworks();
    MeasurementNetwork *createNetwork();
    void removeNetwork(MeasurementNetwork *network);

    void cloneFrom(MeasurementSetup &origin);
    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    bool loadXML(Backend &backend, QDomElement &el);

signals:
    void onSetupChanged();

private:
    QList<MeasurementNetwork*> _networks;
};
