/*
 * SocketCanInterface.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef SOCKETCAN_SOCKETCANINTERFACE_H_
#define SOCKETCAN_SOCKETCANINTERFACE_H_

#include "../CanInterface.h"

class SocketCanInterfaceProvider;

class SocketCanInterface: public CanInterface {
public:
	SocketCanInterface(SocketCanInterfaceProvider *provider, int index, string name);
	virtual ~SocketCanInterface();

	virtual string getName();
	void setName(string name);

	virtual int getBitrate();
	virtual void setBitrate(int bitrate);

	virtual void open();
	virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(CanMessage &msg);

	int getIfIndex();


private:
	int _idx;
	int _fd;
	string _name;
};

#endif /* SOCKETCAN_SOCKETCANINTERFACE_H_ */
