/*
 * SocketCanInterface.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "SocketCanInterface.h"
#include "model/CanMessage.h"
#include "libsocketcan/libsocketcan.h"

#include <stdio.h>
#include <unistd.h>
#include <QString>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <linux/can.h>
#include <linux/can/raw.h>

SocketCanInterface::SocketCanInterface(SocketCanDriver *driver, int index, QString name)
  : CanInterface((CanDriver *)driver),
	_idx(index),
	_fd(0),
	_name(name)
{
}

SocketCanInterface::~SocketCanInterface() {
}

QString SocketCanInterface::getName() const {
	return _name;
}

void SocketCanInterface::setName(QString name) {
	_name = name;
}

int SocketCanInterface::getBitrate() {
	struct can_bittiming bt;
    if (can_get_bittiming(_name.toStdString().c_str(), &bt) == 0) {
		return bt.bitrate;
	} else {
		return 0;
	}
}

void SocketCanInterface::setBitrate(int bitrate) {
    can_set_bitrate(_name.toStdString().c_str(), bitrate);
}

int SocketCanInterface::getIfIndex() {
	return _idx;
}

void SocketCanInterface::open() {
	if((_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
	}

	struct ifreq ifr;
    struct sockaddr_can addr;
    strcpy(ifr.ifr_name, _name.toStdString().c_str());
	ioctl(_fd, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if(bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
	}
}

void SocketCanInterface::close() {
	::close(_fd);
}

void SocketCanInterface::sendMessage(const CanMessage &msg) {
	struct can_frame frame;

	frame.can_id = msg.getId();

	if (msg.isExtended()) {
		frame.can_id |= CAN_EFF_FLAG;
	}

	if (msg.isRTR()) {
		frame.can_id |= CAN_RTR_FLAG;
	}

	if (msg.isErrorFrame()) {
		frame.can_id |= CAN_ERR_FLAG;
	}

	uint8_t len = msg.getLength();
	if (len>8) { len = 8; }

	frame.can_dlc = len;
	for (int i=0; i<len; i++) {
		frame.data[i] = msg.getByte(i);
	}

	::write(_fd, &frame, sizeof(struct can_frame));
}

bool SocketCanInterface::readMessage(CanMessage &msg, unsigned int timeout_ms) {

    struct can_frame frame;
    struct timeval tv_rcv;
    struct timeval timeout;
    fd_set fdset;

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = 1000 * (timeout_ms % 1000);

    FD_ZERO(&fdset);
    FD_SET(_fd, &fdset);

    int rv = select(_fd+1, &fdset, NULL, NULL, &timeout);
    if (rv>0) {
        ::read(_fd, &frame, sizeof(struct can_frame));

        ioctl(_fd, SIOCGSTAMP, &tv_rcv);

        msg.setId(frame.can_id);
        msg.setTimestamp(tv_rcv);
        msg.setExtended((frame.can_id & CAN_EFF_FLAG)!=0);
        msg.setRTR((frame.can_id & CAN_RTR_FLAG)!=0);
        msg.setErrorFrame((frame.can_id & CAN_ERR_FLAG)!=0);
        msg.setInterface(this);

        uint8_t len = frame.can_dlc;
        if (len>8) { len = 8; }

        msg.setLength(len);
        for (int i=0; i<len; i++) {
            msg.setByte(i, frame.data[i]);
        }

        return true;
    } else {
        return false;
    }
}
