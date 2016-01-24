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

#include "FilterSet.h"
#include "FilterItem.h"
#include "FilterSetup.h"

FilterSet::FilterSet(FilterSetup *parent)
  : QObject(parent), _isActive(true), _accept(this), _drop(this)
{
}

bool FilterSet::acceptsMessage(CanMessage &msg)
{
    if (!_isActive) {
        return true;
    }

    for (int i=0; i<_drop.size(); i++) {
        FilterItem *item = _drop.at(i);
        if (item->matchesMessage(msg)) {
            return false;
        }
    }

    for (int i=0; i<_accept.size(); i++) {
        FilterItem *item = _accept.at(i);
        if (item->matchesMessage(msg)) {
            return true;
        }
    }

    if (_accept.empty()) {
        return true;
    } else if (_drop.empty()) {
        return false;
    } else {
        return true;
    }

}

FilterList &FilterSet::acceptList()
{
    return _accept;
}

FilterList &FilterSet::dropList()
{
    return _drop;
}
