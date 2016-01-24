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

#include "CanDb.h"
#include <QFileInfo>
#include <QDomDocument>

#include <core/Backend.h>

CanDb::CanDb()
{

}

QString CanDb::getFileName()
{
    QFileInfo fi(getPath());
    return fi.fileName();
}

QString CanDb::getDirectory()
{
    QFileInfo fi(getPath());
    return fi.absolutePath();
}

CanDbNode *CanDb::getOrCreateNode(QString node_name)
{
    if (!_nodes.contains(node_name)) {
        CanDbNode *node = new CanDbNode(this);
        node->setName(node_name);
        _nodes[node_name] = node;
        return node;
    } else {
        return _nodes[node_name];
    }
}

CanDbMessage *CanDb::getMessageById(uint32_t raw_id)
{
    if (_messages.contains(raw_id)) {
        return _messages[raw_id];
    } else {
        return 0;
    }
}

void CanDb::addMessage(CanDbMessage *msg)
{
    _messages[msg->getRaw_id()] = msg;
}

QString CanDb::getComment() const
{
    return _comment;
}

void CanDb::setComment(const QString &comment)
{
    _comment = comment;
}

bool CanDb::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    (void) backend;
    (void) xml;
    root.setAttribute("type", "dbc");
    root.setAttribute("filename", _path);
    return true;
}

