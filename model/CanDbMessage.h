#ifndef CANDBMESSAGE_H
#define CANDBMESSAGE_H

#include <stdint.h>
#include <QString>
#include "CanDb.h"
#include "CanDbSignal.h"

class CanDbNode;
class CanDbSignal;

typedef QList<CanDbSignal*> CanDbSignalList;

class CanDbMessage
{
    public:
        CanDbMessage(CanDb *parent);

        QString getName() const;
        void setName(const QString &name);

        uint32_t getRaw_id() const;
        void setRaw_id(const uint32_t &raw_id);

        uint8_t getDlc() const;
        void setDlc(const uint8_t &dlc);

        CanDbNode *getSender() const;
        void setSender(CanDbNode *sender);

        void addSignal(CanDbSignal *signal);
        CanDbSignal *getSignal(int num);

        CanDbSignalList getSignals() { return _signals; }

private:
        CanDb *_parent;
        QString _name;
        uint32_t _raw_id;
        uint8_t _dlc;
        CanDbNode *_sender;
        CanDbSignalList _signals;

};

#endif // CANDBMESSAGE_H
