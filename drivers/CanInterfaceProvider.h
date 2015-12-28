/*
 * CanInterfaceProvider.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef BASE_CANINTERFACEPROVIDER_H_
#define BASE_CANINTERFACEPROVIDER_H_

#include <QList>
#include "CanInterface.h"

using namespace std;

class CanInterface;

typedef QList<CanInterface*> CanInterfaceList;

class CanInterfaceProvider {
public:
	CanInterfaceProvider();
	virtual ~CanInterfaceProvider();

	virtual string getName() = 0;
	virtual CanInterfaceList getInterfaceList() = 0;
	virtual void update() = 0;
};

#endif /* BASE_CANINTERFACEPROVIDER_H_ */
