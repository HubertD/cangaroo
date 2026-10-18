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

#include "SocketCanInterface.h"

#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>
#include <core/CanMessage.h>

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/netlink.h>
#include <linux/sockios.h>
#include <netlink/version.h>
#include <netlink/route/link.h>
#include <netlink/route/link/can.h>


SocketCanInterface::SocketCanInterface(SocketCanDriver *driver, int index, QString name)
  : CanInterface((CanDriver *)driver),
	_idx(index),
    _isOpen(false),
	_fd(0),
    _name(name),
    _ts_mode(ts_mode_SIOCSHWTSTAMP)
{
    _status.rx_count = 0;
    _status.rx_errors = 0;
    _status.rx_overruns = 0;
    _status.tx_count = 0;
    _status.tx_errors = 0;
    _status.tx_dropped = 0;

    _status.tx_dropped = 0;

    memset(&_config, 0, sizeof(_config));
    memset(&_offset_stats, 0, sizeof(_offset_stats));

    _ts_mode = ts_mode_SIOCGSTAMP;
}

SocketCanInterface::~SocketCanInterface() {
}

QString SocketCanInterface::getName() const {
	return _name;
}

void SocketCanInterface::setName(QString name) {
    _name = name;
}

QList<CanTiming> SocketCanInterface::getAvailableBitrates()
{
    QList<CanTiming> retval;
    QList<unsigned> bitrates({10000, 20000, 50000, 83333, 100000, 125000, 250000, 500000, 800000, 1000000});
    QList<unsigned> samplePoints({500, 625, 750, 875});

    unsigned i=0;
    foreach (unsigned br, bitrates) {
        foreach (unsigned sp, samplePoints) {
            retval << CanTiming(i++, br, 0, sp);
        }
    }

    // Add CAN FD Data Bitrates if supported
    if (supportsCanFD()) {
        QList<unsigned> fdBitrates({500000, 1000000, 2000000, 4000000, 5000000, 8000000});
        foreach (unsigned br, bitrates) {
            foreach (unsigned fdbr, fdBitrates) {
                // For simplicity, we add FD variants of common arbitration bitrates
                retval << CanTiming(i++, br, fdbr, 800, 800);
            }
        }
    }

    return retval;
}

QString SocketCanInterface::buildIpRouteCmd(const MeasurementInterface &mi)
{
    QStringList cmd;
    cmd.append("ip");
    cmd.append("link");
    cmd.append("set");
    cmd.append(getName());
    cmd.append("up");
    cmd.append("type");
    cmd.append("can");

    cmd.append("bitrate");
    cmd.append(QString().number(mi.bitrate()));
    cmd.append("sample-point");
    cmd.append(QString().number((float)mi.samplePoint()/1000.0, 'f', 3));

    if (mi.isCanFD()) {
        cmd.append("dbitrate");
        cmd.append(QString().number(mi.fdBitrate()));
        cmd.append("dsample-point");
        cmd.append(QString().number((float)mi.fdSamplePoint()/1000.0, 'f', 3));
        cmd.append("fd");
        cmd.append("on");
    }

    cmd.append("restart-ms");
    if (mi.doAutoRestart()) {
        cmd.append(QString().number(mi.autoRestartMs()));
    } else {
        cmd.append("0");
    }

    return cmd.join(' ');
}

QStringList SocketCanInterface::buildCanIfConfigArgs(const MeasurementInterface &mi)
{
    QStringList args;
    args << "-d";
    args << "-i" << getName();
    args << "-b" << QString::number(mi.bitrate());
    args << "-p" << QString::number(mi.samplePoint());
    args << "-u";
    return args;
}


void SocketCanInterface::applyConfig(const MeasurementInterface &mi)
{
    if (!mi.doConfigure()) {
        log_info(QString("interface %1 not managed by cangaroo, not touching configuration").arg(getName()));
        return;
    }

    QString cmd;
    QStringList args;

    // Use ip link if CAN FD is requested, as it's the standard way now
    if (mi.isCanFD()) {
        log_info(QString("calling ip link to reconfigure interface %1 (CAN FD)").arg(getName()));
        
        // First bring interface down
        QProcess::execute("ip", {"link", "set", getName(), "down"});
        
        cmd = "ip";
        args << "link" << "set" << getName() << "up" << "type" << "can";
        args << "bitrate" << QString::number(mi.bitrate());
        args << "sample-point" << QString::number((float)mi.samplePoint()/1000.0, 'f', 3);
        args << "dbitrate" << QString::number(mi.fdBitrate());
        args << "dsample-point" << QString::number((float)mi.fdSamplePoint()/1000.0, 'f', 3);
        args << "fd" << "on";
        
        if (mi.doAutoRestart()) {
            args << "restart-ms" << QString::number(mi.autoRestartMs());
        }

    } else {
        log_info(QString("calling canifconfig to reconfigure interface %1").arg(getName()));
        cmd = "canifconfig";
        args = buildCanIfConfigArgs(mi);
    }

    log_info(cmd + " " + args.join(" "));

    QProcess proc;
    proc.start(cmd, args);
    if (!proc.waitForFinished()) {
        log_error(QString("timeout waiting for %1").arg(cmd));
        return;
    }

    if (proc.exitStatus()!=QProcess::NormalExit) {
        log_error(QString("%1 crashed").arg(cmd));
        return;
    }

    if (proc.exitCode() != 0) {
        log_error(QString("%1 failed: ").arg(cmd) + QString(proc.readAllStandardError()).trimmed());
        return;
    }
}

#if (LIBNL_CURRENT<=216)
#warning we need at least libnl3 version 3.2.22 to be able to get link status via netlink
int rtnl_link_can_state(struct rtnl_link *link, uint32_t *state) {
    (void) link;
    (void) state;
    return -1;
}
#endif

bool SocketCanInterface::updateStatus()
{
    bool retval = false;

    struct nl_sock *sock = nl_socket_alloc();
    struct nl_cache *cache = NULL;
    struct rtnl_link *link;
    uint32_t state;

    _status.can_state = state_unknown;

    nl_connect(sock, NETLINK_ROUTE);
    if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache) >= 0) {
        if (rtnl_link_get_kernel(sock, _idx, 0, &link) == 0) {

            _status.rx_count = rtnl_link_get_stat(link, RTNL_LINK_RX_PACKETS);
            _status.rx_overruns = rtnl_link_get_stat(link, RTNL_LINK_RX_OVER_ERR);
            _status.tx_count = rtnl_link_get_stat(link, RTNL_LINK_TX_PACKETS);
            _status.tx_dropped = rtnl_link_get_stat(link, RTNL_LINK_TX_DROPPED);

            if (rtnl_link_is_can(link)) {
                if (rtnl_link_can_state(link, &state)==0) {
                    _status.can_state = state;
                }
                _status.rx_errors = rtnl_link_can_berr_rx(link);
                _status.tx_errors = rtnl_link_can_berr_tx(link);
            } else {
                _status.rx_errors = 0;
                _status.tx_errors = 0;
            }
            retval = true;
        }
    }

    if (cache) {
        nl_cache_free(cache);
    }
    nl_close(sock);
    nl_socket_free(sock);

    return retval;
}

bool SocketCanInterface::readConfig()
{
    bool retval = false;

    struct nl_sock *sock = nl_socket_alloc();
    struct nl_cache *cache = NULL;
    struct rtnl_link *link;

    nl_connect(sock, NETLINK_ROUTE);
    int result = rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache);

    if (result>=0) {
        if (rtnl_link_get_kernel(sock, _idx, 0, &link) == 0) {
            retval = readConfigFromLink(link);
        }
    }

    if (cache) {
        nl_cache_free(cache);
    }
    nl_close(sock);
    nl_socket_free(sock);

    return retval;
}

bool SocketCanInterface::readConfigFromLink(rtnl_link *link)
{
    _config.state = state_unknown;
    _config.supports_canfd = (rtnl_link_get_mtu(link)==72);
    _config.supports_timing = rtnl_link_is_can(link);
    if (_config.supports_timing) {
        rtnl_link_can_freq(link, &_config.base_freq);
        rtnl_link_can_get_ctrlmode(link, &_config.ctrl_mode);
        rtnl_link_can_get_bittiming(link, &_config.bit_timing);
        rtnl_link_can_get_sample_point(link, &_config.sample_point);
        rtnl_link_can_get_restart_ms(link, &_config.restart_ms);
    } else {
        // maybe a vcan interface?
    }
    return true;
}

bool SocketCanInterface::supportsTimingConfiguration()
{
    return _config.supports_timing;
}

bool SocketCanInterface::supportsCanFD()
{
    return _config.supports_canfd;
}

bool SocketCanInterface::supportsTripleSampling()
{
    return false;
}

unsigned SocketCanInterface::getBitrate() {
    unsigned br = 0;
    if (readConfig()) {
        br = _config.bit_timing.bitrate;
    }

    if (br == 0) {
        // Fallback to setup bitrate
        foreach (MeasurementNetwork *network, Backend::instance().getSetup().getNetworks()) {
            foreach (MeasurementInterface *mi, network->interfaces()) {
                if (mi->canInterface() == getId()) {
                    unsigned fallbackBr = mi->bitrate();
                    // log_info(QString("SocketCanInterface %1: getBitrate() fallback to %2 (ID match %3)").arg(_name).arg(fallbackBr).arg(getId()));
                    return fallbackBr;
                }
            }
        }
    }

    if (br == 0) br = 500000; // Final safety fallback

    return br;
}

uint32_t SocketCanInterface::getCapabilities()
{
    uint32_t retval =
        CanInterface::capability_config_os |
        CanInterface::capability_listen_only |
        CanInterface::capability_auto_restart;

    if (supportsCanFD()) {
        retval |= CanInterface::capability_canfd;
    }

    if (supportsTripleSampling()) {
        retval |= CanInterface::capability_triple_sampling;
    }

    return retval;
}

bool SocketCanInterface::updateStatistics()
{
    return updateStatus();
}

void SocketCanInterface::resetStatistics()
{
    _offset_stats = _status;
    CanInterface::resetStatistics();
}

uint32_t SocketCanInterface::getState()
{
    switch (_status.can_state) {
        case CAN_STATE_ERROR_ACTIVE: return state_ok;
        case CAN_STATE_ERROR_WARNING: return state_warning;
        case CAN_STATE_ERROR_PASSIVE: return state_passive;
        case CAN_STATE_BUS_OFF: return state_bus_off;
        case CAN_STATE_STOPPED: return state_stopped;
        default: return state_unknown;
    }
}

int SocketCanInterface::getNumRxFrames()
{
    return _status.rx_count - _offset_stats.rx_count;
}

int SocketCanInterface::getNumRxErrors()
{
    return _status.rx_errors - _offset_stats.rx_errors;
}

int SocketCanInterface::getNumTxFrames()
{
    return _status.tx_count - _offset_stats.tx_count;
}

int SocketCanInterface::getNumTxErrors()
{
    return _status.tx_errors - _offset_stats.tx_errors;
}

int SocketCanInterface::getNumRxOverruns()
{
    return _status.rx_overruns - _offset_stats.rx_overruns;
}

int SocketCanInterface::getNumTxDropped()
{
    return _status.tx_dropped - _offset_stats.tx_dropped;
}

int SocketCanInterface::getIfIndex() {
    return _idx;
}

const char *SocketCanInterface::cname()
{
    return _name.toStdString().c_str();
}

void SocketCanInterface::open() {
    _isOpen = false;
    if((_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        log_error(QString("SocketCanInterface: Error while opening socket: %1").arg(strerror(errno)));
        return;
    }

    struct ifreq ifr;
    struct sockaddr_can addr;

    strncpy(ifr.ifr_name, _name.toStdString().c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(_fd, SIOCGIFINDEX, &ifr) < 0) {
        log_error(QString("SocketCanInterface: Error getting interface index for %1: %2").arg(_name, strerror(errno)));
        ::close(_fd);
        return;
    }

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if(bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        log_error(QString("SocketCanInterface: Error in socket bind for %1: %2").arg(_name, strerror(errno)));
        ::close(_fd);
        return;
    }

    int enable = 1;
    if (setsockopt(_fd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable, sizeof(enable)) != 0) {
        // Just info, not error, since older kernels might not support it
        // and we can still communicate using classic CAN frames.
    }

    _isOpen = true;
}

bool SocketCanInterface::isOpen()
{
    return _isOpen;
}

void SocketCanInterface::close() {
    ::close(_fd);
    _isOpen = false;
}

void SocketCanInterface::sendMessage(const CanMessage &msg) {
    if (!_isOpen) {
        log_error(QString("SocketCanInterface: Cannot send message, interface %1 is not open").arg(_name));
        return;
    }

    if (msg.isFD() || (supportsCanFD() && msg.getLength() > 8)) {
        struct canfd_frame frame;
        memset(&frame, 0, sizeof(frame));
        frame.can_id = msg.getId();

        if (msg.isExtended()) {
            frame.can_id |= CAN_EFF_FLAG;
        }

        if (msg.isErrorFrame()) {
            frame.can_id |= CAN_ERR_FLAG;
        }

        if (msg.isBRS()) {
            frame.flags |= CANFD_BRS;
        }

        uint8_t len = msg.getLength();
        if (len > 64) len = 64;
        frame.len = len;

        for (int i=0; i<len; i++) {
            frame.data[i] = msg.getByte(i);
        }

        if (::write(_fd, &frame, sizeof(struct canfd_frame)) < 0) {
            log_error(QString("SocketCanInterface: Error writing FD frame to %1: %2").arg(_name, strerror(errno)));
        }
    } else {
        struct can_frame frame;
        memset(&frame, 0, sizeof(frame));
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
        if (len > 8) len = 8;
        frame.can_dlc = len;

        for (int i=0; i<len; i++) {
            frame.data[i] = msg.getByte(i);
        }

        if (::write(_fd, &frame, sizeof(struct can_frame)) < 0) {
            log_error(QString("SocketCanInterface: Error writing frame to %1: %2").arg(_name, strerror(errno)));
        }
    }
    // Track sent bits
    addFrameBits(msg);
}

bool SocketCanInterface::readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms) {

    struct canfd_frame frame;
    struct timespec ts_rcv;
    struct timeval tv_rcv;
    struct timeval timeout;
    fd_set fdset;

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = 1000 * (timeout_ms % 1000);

    FD_ZERO(&fdset);
    FD_SET(_fd, &fdset);

    CanMessage msg;

    int rv = select(_fd+1, &fdset, NULL, NULL, &timeout);
    if (rv > 0) {
        int nbytes = ::read(_fd, &frame, sizeof(struct canfd_frame));
        if (nbytes < 0) {
            return false;
        }

        if (_ts_mode == ts_mode_SIOCSHWTSTAMP) {
            // TODO implement me
            _ts_mode = ts_mode_SIOCGSTAMPNS;
        }

        if (_ts_mode == ts_mode_SIOCGSTAMPNS) {
            if (ioctl(_fd, SIOCGSTAMPNS, &ts_rcv) == 0) {
                msg.setTimestamp(ts_rcv.tv_sec, ts_rcv.tv_nsec/1000);
            } else {
                _ts_mode = ts_mode_SIOCGSTAMP;
            }
        }

        if (_ts_mode == ts_mode_SIOCGSTAMP) {
            ioctl(_fd, SIOCGSTAMP, &tv_rcv);
            msg.setTimestamp(tv_rcv.tv_sec, tv_rcv.tv_usec);
        }

        msg.setId(frame.can_id & CAN_EFF_MASK);
        msg.setExtended((frame.can_id & CAN_EFF_FLAG)!=0);
        msg.setRTR((frame.can_id & CAN_RTR_FLAG)!=0);
        msg.setErrorFrame((frame.can_id & CAN_ERR_FLAG)!=0);
        msg.setInterfaceId(getId());

        bool isFD = (nbytes == CANFD_MTU);
        msg.setFD(isFD);
        if (isFD) {
            msg.setBRS((frame.flags & CANFD_BRS) != 0);
        }

        uint8_t len = frame.len;
        msg.setLength(len);
        for (int i=0; i<len; i++) {
            msg.setByte(i, frame.data[i]);
        }

        msglist.append(msg);
        addFrameBits(msg); // Track received bits
        return true;
    } 
    
    return false;
}
