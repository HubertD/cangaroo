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

#include "AggregatedTraceViewItem.h"

AggregatedTraceViewItem::AggregatedTraceViewItem(AggregatedTraceViewItem *parent)
  : _parent(parent)
{
}

AggregatedTraceViewItem::~AggregatedTraceViewItem()
{
    qDeleteAll(_children);
}

void AggregatedTraceViewItem::appendChild(AggregatedTraceViewItem *child)
{
    _children.append(child);
}

AggregatedTraceViewItem *AggregatedTraceViewItem::child(int row) const
{
    return _children.value(row);
}

int AggregatedTraceViewItem::childCount() const
{
    return _children.count();
}

int AggregatedTraceViewItem::row() const
{
    if (_parent) {
        return _parent->_children.indexOf(const_cast<AggregatedTraceViewItem*>(this));
    } else {
        return 0;
    }
}

AggregatedTraceViewItem *AggregatedTraceViewItem::parent() const
{
    return _parent;
}

AggregatedTraceViewItem *AggregatedTraceViewItem::firstChild() const
{
    return _children.first();
}

AggregatedTraceViewItem *AggregatedTraceViewItem::lastChild() const
{
    return _children.last();
}
