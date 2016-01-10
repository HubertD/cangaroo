#ifndef CANDBSIGNAL_H
#define CANDBSIGNAL_H

#include "CanMessage.h"
#include "CanDbMessage.h"
#include <QString>
#include <QMap>

class CanDbMessage;

typedef QMap<uint32_t,QString> CanDbValueTable;

class CanDbSignal
{
public:
    CanDbSignal(CanDbMessage *parent);
    QString name() const;
    void setName(const QString &name);

    int startBit() const;
    void setStartBit(int startBit);

    int length() const;
    void setLength(int length);

    QString comment() const;
    void setComment(const QString &comment);

    QString getValueName(const uint32_t value) const;
    void setValueName(const uint32_t value, const QString &name);

    double convertRawValueToPhysical(const uint32_t rawValue);

    double getFactor() const;
    void setFactor(double factor);

    double getOffset() const;
    void setOffset(double offset);

    double getMinimumValue() const;
    void setMinimumValue(double min);

    double getMaximumValue() const;
    void setMaximumValue(double max);

    bool isUnsigned() const;
    void setUnsigned(bool isUnsigned);

private:
    CanDbMessage *_parent;
    QString _name;
    int _startBit;
    int _length;
    bool _isUnsigned;
    double _factor;
    double _offset;
    double _min;
    double _max;
    QString _comment;
    CanDbValueTable _valueTable;
};

#endif // CANDBSIGNAL_H
