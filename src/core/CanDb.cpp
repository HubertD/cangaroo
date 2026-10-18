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

size_t CanDb::getNumberOfMessages()
{
    return _messages.size();
}

CanDbMessage *CanDb::getMessageById(uint32_t raw_id)
{
    if (_messages.contains(raw_id)) {
        return _messages[raw_id];
    } else {
        return 0;
    }
}

CanDbMessageList CanDb::getMessageList()
{
    return _messages;
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

void CanDb::updateFrom(CanDb *other)
{
    this->setVersion(other->getVersion());
    this->setComment(other->getComment());

    for (CanDbMessage *otherMsg : other->getMessageList()) {
        CanDbMessage *myMsg = this->getMessageById(otherMsg->getRaw_id());
        if (!myMsg) {
            myMsg = new CanDbMessage(this);
            myMsg->setName(otherMsg->getName());
            myMsg->setRaw_id(otherMsg->getRaw_id());
            myMsg->setDlc(otherMsg->getDlc());
            myMsg->setComment(otherMsg->getComment());
            this->addMessage(myMsg);
        } else {
            myMsg->setName(otherMsg->getName());
            myMsg->setDlc(otherMsg->getDlc());
            myMsg->setComment(otherMsg->getComment());
        }

        for (CanDbSignal *otherSig : otherMsg->getSignals()) {
            CanDbSignal *mySig = myMsg->getSignalByName(otherSig->name());
            if (!mySig) {
                mySig = new CanDbSignal(myMsg);
                mySig->setName(otherSig->name());
                myMsg->addSignal(mySig);
            }
            mySig->setStartBit(otherSig->startBit());
            mySig->setLength(otherSig->length());
            mySig->setFactor(otherSig->getFactor());
            mySig->setOffset(otherSig->getOffset());
            mySig->setMinimumValue(otherSig->getMinimumValue());
            mySig->setMaximumValue(otherSig->getMaximumValue());
            mySig->setUnit(otherSig->getUnit());
            mySig->setUnsigned(otherSig->isUnsigned());
            mySig->setIsBigEndian(otherSig->isBigEndian());
            mySig->setIsMuxer(otherSig->isMuxer());
            mySig->setIsMuxed(otherSig->isMuxed());
            mySig->setMuxValue(otherSig->getMuxValue());
            mySig->setComment(otherSig->comment());
        }
    }
}

