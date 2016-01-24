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


#include "FilterList.h"
#include "FilterItem.h"

FilterList::FilterList(QObject *parent)
  : QObject(parent)
{
}

int FilterList::size() const
{
    return children().size();
}

FilterItem *FilterList::at(int index) const
{
    return (FilterItem*)children().at(index);
}

bool FilterList::contains(FilterItem *item)
{
    return children().contains(item);
}

int FilterList::indexOf(FilterItem *item)
{
    return children().indexOf(item);
}

bool FilterList::empty() const
{
    return children().empty();
}

void FilterList::addItem(FilterItem *item)
{
    item->setParent(this);
}

bool FilterList::matchesMessage(CanMessage &msg)
{
    foreach (QObject *obj, children()) {
        FilterItem *item = dynamic_cast<FilterItem*>(obj);
        if (item->matchesMessage(msg)) {
            return true;
        }
    }

    return false;
}
