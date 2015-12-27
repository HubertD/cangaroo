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

private:
    CanDb *_parent;
    QString _name;
};

#endif // CANDBNODE_H
