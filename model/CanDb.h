#ifndef CANDB_H
#define CANDB_H

#include <QString>
#include <QList>
#include <QMap>
#include "CanDbNode.h"
#include "CanDbMessage.h"

class CanDbMessage;

typedef QMap<QString,CanDbNode*> CanDbNodeMap;
typedef QMap<uint32_t, CanDbMessage*> CanDbMessageList;

class CanDb
{
    public:
        CanDb();

        void setFilename(QString filename) { _filename = filename; }
        QString getFilename() { return _filename; }
        QString getBaseFilename();

        void setVersion(QString version) { _version = version; }
        QString getVersion() { return _version; }

        CanDbNode *getOrCreateNode(QString node_name);

        CanDbMessage *getMessageById(uint32_t raw_id);
        void addMessage(CanDbMessage *msg);

        QString getComment() const;
        void setComment(const QString &comment);

private:
        QString _filename;
        QString _version;
        QString _comment;
        CanDbNodeMap _nodes;
        CanDbMessageList _messages;

};

#endif // CANDB_H
