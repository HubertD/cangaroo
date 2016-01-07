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

typedef QList<pCanInterface> CanInterfaceList;

class CanDriver {
public:
    CanDriver();
    virtual ~CanDriver();

    virtual QString getName() = 0;
	virtual CanInterfaceList getInterfaceList() = 0;
	virtual void update() = 0;
};

#endif /* BASE_CANDRIVER_H_ */
