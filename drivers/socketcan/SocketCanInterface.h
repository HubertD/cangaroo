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
    SocketCanInterface(SocketCanInterfaceProvider *provider, int index, QString name);
	virtual ~SocketCanInterface();

    virtual QString getName();
    void setName(QString name);

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
    QString _name;
};

#endif /* SOCKETCAN_SOCKETCANINTERFACE_H_ */
