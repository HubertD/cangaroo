#include "CanDbSignal.h"

CanDbSignal::CanDbSignal(CanDbMessage *parent)
  : _parent(parent),
    _isUnsigned(false),
    _factor(1),
    _offset(0),
    _min(0),
    _max(0)
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

QString CanDbSignal::comment() const
{
    return _comment;
}

void CanDbSignal::setComment(const QString &comment)
{
    _comment = comment;
}

QString CanDbSignal::getValueName(const uint32_t value) const
{
    if (_valueTable.contains(value)) {
        return _valueTable[value];
    } else {
        return QString();
    }
}

void CanDbSignal::setValueName(const uint32_t value, const QString &name)
{
    _valueTable[value] = name;
}

double CanDbSignal::convertRawValueToPhysical(const uint32_t rawValue)
{
    int v;
    if (isUnsigned()) {
        v = rawValue;
    } else {
        // TODO check with DBC that actually contains signed values?!
        v = (int32_t)(rawValue<<(32-_length));
        v>>=(32-_length);
    }
    return v * _factor + _offset;
}

double CanDbSignal::getFactor() const
{
    return _factor;
}

void CanDbSignal::setFactor(double factor)
{
    _factor = factor;
}

double CanDbSignal::getOffset() const
{
    return _offset;
}

void CanDbSignal::setOffset(double offset)
{
    _offset = offset;
}

double CanDbSignal::getMinimumValue() const
{
    return _min;
}

void CanDbSignal::setMinimumValue(double min)
{
    _min = min;
}

double CanDbSignal::getMaximumValue() const
{
    return _max;
}

void CanDbSignal::setMaximumValue(double max)
{
    _max = max;
}

bool CanDbSignal::isUnsigned() const
{
    return _isUnsigned;
}

void CanDbSignal::setUnsigned(bool isUnsigned)
{
    _isUnsigned = isUnsigned;
}

