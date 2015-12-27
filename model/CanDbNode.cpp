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

