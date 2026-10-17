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
#include <QMap>
#include <QSharedPointer>

#include "CanDbNode.h"
#include "CanDbMessage.h"

class QDomDocument;
class QDomElement;

class Backend;
class CanDb;
class CanDbMessage;

typedef QMap<QString,CanDbNode*> CanDbNodeMap;
typedef QMap<uint32_t, CanDbMessage*> CanDbMessageList;
typedef QSharedPointer<CanDb> pCanDb;

class CanDb
{
    public:
        CanDb();

        void setPath(QString path) { _path = path; }
        QString getPath() { return _path; }
        QString getFileName();
        QString getDirectory();

        void setVersion(QString version) { _version = version; }
        QString getVersion() { return _version; }

        void setJ1939(bool v) { _isJ1939 = v; }
        bool isJ1939() const { return _isJ1939; }

        CanDbNode *getOrCreateNode(QString node_name);

        size_t getNumberOfMessages();

        CanDbMessage *getMessageById(uint32_t raw_id);
        CanDbMessageList getMessageList();

        void addMessage(CanDbMessage *msg);

        QString getComment() const;
        void setComment(const QString &comment);

        bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

        void updateFrom(CanDb *other);

private:
        QString _path;
        QString _version;
        QString _comment;
        bool _isJ1939 = false;
        CanDbNodeMap _nodes;
        CanDbMessageList _messages;

};
