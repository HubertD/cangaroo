/*
 * CanDriver.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef BASE_CANDRIVER_H_
#define BASE_CANDRIVER_H_

#include <QList>
#include <QString>
#include "CanInterface.h"

class CanInterface;

typedef uint16_t CanInterfaceId;
typedef QList<uint16_t> CanInterfaceIdList;

class CanDriver {
public:
    CanDriver();
    virtual ~CanDriver();

    void setId(int id);
    int id();

    virtual QString getName() = 0;
	virtual void update() = 0;

    CanInterfaceIdList getInterfaceIds() const;
    QList<CanInterface*> getInterfaces() const;
    CanInterface *getInterfaceById(CanInterfaceId id);
    CanInterfaceId addInterface(CanInterface *intf);

    CanInterface *getInterfaceByName(QString ifName);

private:
    int _id;
    QList<CanInterface*> _interfaces;
};

#endif /* BASE_CANDRIVER_H_ */
