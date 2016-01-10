/*
 * SocketCanDriver.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef SOCKETCAN_SOCKETCANDRIVER_H_
#define SOCKETCAN_SOCKETCANDRIVER_H_

#include <sys/types.h>
#include <linux/netlink.h>
#include <QString>
#include <driver/CanDriver.h>

class SocketCanDriver: public CanDriver {
public:
    SocketCanDriver();
    virtual ~SocketCanDriver();

    virtual QString getName();
	virtual void update();

private:
	void addNetlinkInterface(struct nlmsghdr *h);
    void createOrUpdateInterface(int index, QString name);
};

#endif /* SOCKETCAN_SOCKETCANDRIVER_H_ */
