/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

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

class FilterItem;
class CanMessage;

class FilterList : public QObject
{
    Q_OBJECT
public:
    explicit FilterList(QObject *parent = 0);

    int size() const;
    FilterItem *at(int index) const;
    bool contains(FilterItem *at);
    int indexOf(FilterItem *at);
    bool empty() const;

    void addItem(FilterItem *at);

    virtual bool matchesMessage(CanMessage &msg);
};
