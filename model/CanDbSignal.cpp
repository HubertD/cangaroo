#include "CanDbSignal.h"

CanDbSignal::CanDbSignal(CanDbMessage *parent)
  : _parent(parent)
{

}

QString CanDbSignal::name() const
{
    return _name;
}

void CanDbSignal::setName(const QString &name)
{
    _name = name;
}

int CanDbSignal::startBit() const
{
    return _startBit;
}

void CanDbSignal::setStartBit(int startBit)
{
    _startBit = startBit;
}

int CanDbSignal::length() const
{
    return _length;
}

void CanDbSignal::setLength(int length)
{
    _length = length;
}

