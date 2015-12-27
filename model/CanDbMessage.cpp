#include "CanDbMessage.h"

CanDbMessage::CanDbMessage(CanDb *parent)
  : _parent(parent), _raw_id(0), _dlc(0), _sender(0)
{
}

QString CanDbMessage::getName() const
{
    return _name;
}

void CanDbMessage::setName(const QString &name)
{
    _name = name;
}

uint32_t CanDbMessage::getRaw_id() const
{
    return _raw_id;
}

void CanDbMessage::setRaw_id(const uint32_t &raw_id)
{
    _raw_id = raw_id;
}

uint8_t CanDbMessage::getDlc() const
{
    return _dlc;
}

void CanDbMessage::setDlc(const uint8_t &dlc)
{
    _dlc = dlc;
}

CanDbNode *CanDbMessage::getSender() const
{
    return _sender;
}

void CanDbMessage::setSender(CanDbNode *sender)
{
    _sender = sender;
}
