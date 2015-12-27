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

private:
    CanDbMessage *_parent;
    QString _name;
    int _startBit;
    int _length;
    QString _comment;
    CanDbValueTable _valueTable;
};

#endif // CANDBSIGNAL_H
