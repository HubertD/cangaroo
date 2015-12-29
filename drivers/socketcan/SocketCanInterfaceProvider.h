/*
 * SocketCanInterfaceProvider.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef SOCKETCAN_SOCKETCANINTERFACEPROVIDER_H_
#define SOCKETCAN_SOCKETCANINTERFACEPROVIDER_H_

#include "../CanInterfaceProvider.h"

#include <sys/types.h>
#include <linux/netlink.h>
#include <QString>

class SocketCanInterfaceProvider: public CanInterfaceProvider {
public:
	SocketCanInterfaceProvider();
	virtual ~SocketCanInterfaceProvider();

    virtual QString getName();
	virtual CanInterfaceList getInterfaceList();
	virtual void update();

private:
	CanInterfaceList _interfaces;

	void addNetlinkInterface(struct nlmsghdr *h);
    void createOrUpdateInterface(int index, QString name);
};

#endif /* SOCKETCAN_SOCKETCANINTERFACEPROVIDER_H_ */
