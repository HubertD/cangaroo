/*
 * SocketCanInterface.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef SOCKETCAN_SOCKETCANINTERFACE_H_
#define SOCKETCAN_SOCKETCANINTERFACE_H_

#include "../CanInterface.h"

class SocketCanDriver;

class SocketCanInterface: public CanInterface {
public:
    SocketCanInterface(SocketCanDriver *driver, int index, QString name);
	virtual ~SocketCanInterface();

    virtual QString getName() const;
    void setName(QString name);

	virtual int getBitrate();
	virtual void setBitrate(int bitrate);

	virtual void open();
	virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(CanMessage &msg, unsigned int timeout_ms);

	int getIfIndex();


private:
	int _idx;
	int _fd;
    QString _name;
};

#endif /* SOCKETCAN_SOCKETCANINTERFACE_H_ */
