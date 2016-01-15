/*

  Copyright (c) 2015, 2016 Hubert Denkmair

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SOCKETCAN_SOCKETCANINTERFACE_H_
#define SOCKETCAN_SOCKETCANINTERFACE_H_

#include "../CanInterface.h"
#include <linux/can/netlink.h>

class SocketCanDriver;

class SocketCanInterface: public CanInterface {
public:
    SocketCanInterface(SocketCanDriver *driver, int index, QString name);
	virtual ~SocketCanInterface();

    virtual QString getName() const;
    void setName(QString name);

    virtual void applyConfig(const MeasurementInterface &mi);

    virtual bool readConfig();
    virtual bool readConfigFromLink(struct rtnl_link *link);

    virtual int getBitrate();
	virtual void setBitrate(int bitrate);

	virtual void open();
	virtual void close();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(CanMessage &msg, unsigned int timeout_ms);

    int getIfIndex();

private:
    typedef enum {
        ts_mode_SIOCSHWTSTAMP,
        ts_mode_SIOCGSTAMPNS,
        ts_mode_SIOCGSTAMP
    } ts_mode_t;

    int _idx;
	int _fd;
    QString _name;

    struct {
        bool supports_canfd;
        bool supports_timing;
        uint32_t state;
        uint32_t base_freq;
        uint32_t sample_point;
        uint32_t ctrl_mode;
        uint32_t restart_ms;
        struct can_bittiming bit_timing;
    } _config;


    ts_mode_t _ts_mode;

    const char *cname();
};

#endif /* SOCKETCAN_SOCKETCANINTERFACE_H_ */
