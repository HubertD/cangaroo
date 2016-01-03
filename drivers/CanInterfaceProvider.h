/*
 * CanInterfaceProvider.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef BASE_CANINTERFACEPROVIDER_H_
#define BASE_CANINTERFACEPROVIDER_H_

#include <QList>
#include <QString>
#include "CanInterface.h"

class CanInterface;

typedef QList<pCanInterface> CanInterfaceList;

class CanInterfaceProvider {
public:
	CanInterfaceProvider();
	virtual ~CanInterfaceProvider();

    virtual QString getName() = 0;
	virtual CanInterfaceList getInterfaceList() = 0;
	virtual void update() = 0;
};

#endif /* BASE_CANINTERFACEPROVIDER_H_ */
