#ifndef CANDBNODE_H
#define CANDBNODE_H

#include <QString>

class CanDb;

class CanDbNode
{
public:
    CanDbNode(CanDb *parent);

    QString name() const;
    void setName(const QString &name);

    QString comment() const;
    void setComment(const QString &comment);

private:
    CanDb *_parent;
    QString _name;
    QString _comment;
};

#endif // CANDBNODE_H
