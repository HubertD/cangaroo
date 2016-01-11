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

#include "CanDbNode.h"

CanDbNode::CanDbNode(CanDb *parent)
  : _parent(parent)
{

}

QString CanDbNode::name() const
{
    return _name;
}

void CanDbNode::setName(const QString &name)
{
    _name = name;
}

QString CanDbNode::comment() const
{
    return _comment;
}

void CanDbNode::setComment(const QString &comment)
{
    _comment = comment;
}

