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

#include <stdint.h>
#include <QList>
#include <QString>

class Backend;
class CanInterface;

typedef uint16_t CanInterfaceId;
typedef QList<uint16_t> CanInterfaceIdList;

class CanDriver {
public:
    CanDriver(Backend &backend);
    virtual ~CanDriver();

    Backend &backend() const;

    void init(int id);

    int id();

    virtual QString getName() = 0;
    virtual bool update() = 0;

    CanInterfaceIdList getInterfaceIds() const;
    QList<CanInterface*> getInterfaces() const;
    CanInterface *getInterfaceById(CanInterfaceId id);
    CanInterfaceId addInterface(CanInterface *intf);
    void deleteInterface(CanInterface *intf);
    void deleteAllInterfaces();

    CanInterface *getInterfaceByName(QString ifName);

private:
    Backend &_backend;
    int _id;
    QList<CanInterface*> _interfaces;

    void setId(int id);
};
