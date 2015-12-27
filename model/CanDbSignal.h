#ifndef CANDBSIGNAL_H
#define CANDBSIGNAL_H

#include "CanMessage.h"
#include "CanDbMessage.h"
#include <QString>

class CanDbMessage;

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

private:
    CanDbMessage *_parent;
    QString _name;
    int _startBit;
    int _length;
};

#endif // CANDBSIGNAL_H
