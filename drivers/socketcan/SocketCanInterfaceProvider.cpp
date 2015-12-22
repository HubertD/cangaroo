/*
 * SocketCanInterfaceProvider.cpp
 *
 *  Created on: 19.12.2015
 *      Author: hd
 */

#include "SocketCanInterfaceProvider.h"
#include "SocketCanInterface.h"

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

SocketCanInterfaceProvider::SocketCanInterfaceProvider()
  : CanInterfaceProvider()
{
}

SocketCanInterfaceProvider::~SocketCanInterfaceProvider() {
}

CanInterfaceList SocketCanInterfaceProvider::getInterfaceList() {
	return _interfaces;
}

void SocketCanInterfaceProvider::update() {
	int fd;
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

	struct sockaddr_nl local;

	pid_t pid = getpid();

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_pid = pid; /* can be obtained using getpid*/
	local.nl_groups = 0;

	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
		fprintf(stderr, "could not bind");
		return;
	}


	typedef struct nl_req_s nl_req_t;

	struct nl_req_s {
	  struct nlmsghdr hdr;
	  struct rtgenmsg gen;
	};

	struct sockaddr_nl kernel;
	struct msghdr rtnl_msg;
	struct iovec io;
	nl_req_t req;

	memset(&rtnl_msg, 0, sizeof(rtnl_msg));
	memset(&kernel, 0, sizeof(kernel));
	memset(&req, 0, sizeof(req));

	kernel.nl_family = AF_NETLINK; /* fill-in kernel address (destination) */


	req.hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	req.hdr.nlmsg_type = RTM_GETLINK;
	req.hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.hdr.nlmsg_seq = 1;
	req.hdr.nlmsg_pid = pid;
	req.gen.rtgen_family = AF_PACKET; /*  no preferred AF, we will get *all* interfaces */

	io.iov_base = &req;
	io.iov_len = req.hdr.nlmsg_len;
	rtnl_msg.msg_iov = &io;
	rtnl_msg.msg_iovlen = 1;
	rtnl_msg.msg_name = &kernel;
	rtnl_msg.msg_namelen = sizeof(kernel);

	sendmsg(fd, (struct msghdr *) &rtnl_msg, 0);



	#define IFLIST_REPLY_BUFFER 8192

	int end = 0;
	char reply[IFLIST_REPLY_BUFFER]; /* a large buffer */
	while (!end) {
		int len;
		struct nlmsghdr *msg_ptr;    /* pointer to current part */
		struct msghdr rtnl_reply;    /* generic msghdr structure */
		struct iovec io_reply;

		memset(&io_reply, 0, sizeof(io_reply));
		memset(&rtnl_reply, 0, sizeof(rtnl_reply));

		io.iov_base = reply;
		io.iov_len = IFLIST_REPLY_BUFFER;
		rtnl_reply.msg_iov = &io;
		rtnl_reply.msg_iovlen = 1;
		rtnl_reply.msg_name = &kernel;
		rtnl_reply.msg_namelen = sizeof(kernel);

		len = recvmsg(fd, &rtnl_reply, 0); /* read lots of data */

		if (len) {
			for (msg_ptr = (struct nlmsghdr *) reply; NLMSG_OK(msg_ptr, len); msg_ptr = NLMSG_NEXT(msg_ptr, len)) {
				switch(msg_ptr->nlmsg_type) {
					case 3:        /* this is the NLMSG_DONE end msg */
						end++;
						break;
					case 16:    /* this is the RTM_NEWLINK msg */
						addNetlinkInterface(msg_ptr);
						break;
					default:    /* for education only, should not happen here */
						printf("message type %d, length %d\n", msg_ptr->nlmsg_type, msg_ptr->nlmsg_len);
						break;
				}
			}
		}
	}

	return;
}

void SocketCanInterfaceProvider::addNetlinkInterface(struct nlmsghdr* h) {
	struct rtattr *attribute;
	int len;

	struct ifinfomsg *iface = (struct ifinfomsg *)NLMSG_DATA(h);
	len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*iface));

	if (iface->ifi_type == 280) { // magic number: can interface
		for (attribute = IFLA_RTA(iface); RTA_OK(attribute, len); attribute = RTA_NEXT(attribute, len)) {
			switch(attribute->rta_type) {
				case IFLA_IFNAME:
					createOrUpdateInterface(iface->ifi_index, string((char *) RTA_DATA(attribute)));
					break;
				default:
					break;
			}
		}
	}
}

string SocketCanInterfaceProvider::getName() {
	return "SocketCAN";
}

void SocketCanInterfaceProvider::createOrUpdateInterface(int index, string name) {

	for (CanInterfaceList::iterator it = _interfaces.begin() ; it != _interfaces.end(); ++it) {
		SocketCanInterface *scif = dynamic_cast<SocketCanInterface *>(*it);
		if (scif->getIfIndex() == index) {
			scif->setName(name);
			return;
		}
	}

	SocketCanInterface *scif = new SocketCanInterface(this, index, name);
	_interfaces.push_back(scif);
}
