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

#include <QString>
#include <QList>
#include <QDomDocument>

#include <driver/CanDriver.h>
#include <driver/CanInterface.h>
#include <core/CanDb.h>

class Backend;
class MeasurementInterface;

class MeasurementNetwork
{
public:
    MeasurementNetwork();
    void cloneFrom(MeasurementNetwork &origin);

    void addInterface(MeasurementInterface *intf);
    void removeInterface(MeasurementInterface *intf);
    QList<MeasurementInterface*> interfaces();

    MeasurementInterface *addCanInterface(CanInterfaceId canif);
    CanInterfaceIdList getReferencedCanInterfaces();

    void addCanDb(pCanDb candb);
    void reloadCanDbs(Backend *backend);
    QList<pCanDb> _canDbs;

    QString name() const;
    void setName(const QString &name);

    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    bool loadXML(Backend &backend, QDomElement el);

private:
    QString _name;
    QList<MeasurementInterface*> _interfaces;
};
