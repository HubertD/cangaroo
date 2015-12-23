/*
 * CanInterface.h
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#ifndef BASE_CANINTERFACE_H_
#define BASE_CANINTERFACE_H_

#include <string>
#include <stdint.h>

using namespace std;

class CanInterfaceProvider;
class CanMessage;

class CanInterface {
public:
	CanInterface(CanInterfaceProvider *provider);
	virtual ~CanInterface();
	virtual CanInterfaceProvider *getProvider();
	virtual string getName() = 0;

	virtual int getBitrate() = 0;
	virtual void setBitrate(int bitrate) = 0;

	virtual void open();
	virtual void close();

	virtual void sendMessage(const CanMessage msg) = 0;
	virtual CanMessage readMessage() = 0;

    uint16_t getId();
    void setId(uint16_t id);

private:
    uint16_t _id;
	CanInterfaceProvider *_provider;
};

#endif /* BASE_CANINTERFACE_H_ */
