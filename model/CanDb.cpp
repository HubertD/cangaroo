#include "CanDb.h"
#include <QFileInfo>
#include <QDomDocument>

#include <Backend.h>

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

