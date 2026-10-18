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

#pragma once

#include "../CanInterface.h"
#include <core/MeasurementInterface.h>
#include <QtNetwork/QUdpSocket>
#include <QTimer>

class CANBlasterDriver;
typedef struct {
    bool supports_canfd;
    bool supports_timing;
    uint32_t state;
    uint32_t base_freq;
    uint32_t sample_point;
    uint32_t ctrl_mode;
    uint32_t restart_ms;
} can_config_t;

typedef struct {
    uint32_t can_state;

    uint64_t rx_count;
    int rx_errors;
    uint64_t rx_overruns;

    uint64_t tx_count;
    int tx_errors;
    uint64_t tx_dropped;
} can_status_t;

class CANBlasterInterface: public CanInterface {
    Q_OBJECT
public:
    CANBlasterInterface(CANBlasterDriver *driver, int index, QString name, bool fd_support);
    virtual ~CANBlasterInterface();

    QString getDetailsStr() const;
    virtual QString getName() const;
    void setName(QString name);

    virtual QList<CanTiming> getAvailableBitrates();

    virtual void applyConfig(const MeasurementInterface &mi);

    bool supportsTimingConfiguration();
    bool supportsCanFD();
    bool supportsTripleSampling();

    virtual unsigned getBitrate();
    virtual uint32_t getCapabilities();


	virtual void open();
    virtual void close();
    virtual bool isOpen();

    virtual void sendMessage(const CanMessage &msg);
    virtual bool readMessage(QList<CanMessage> &msglist, unsigned int timeout_ms);

    virtual bool updateStatistics();
    virtual uint32_t getState();
    virtual int getNumRxFrames();
    virtual int getNumRxErrors();
    virtual int getNumRxOverruns();

    virtual int getNumTxFrames();
    virtual int getNumTxErrors();
    virtual int getNumTxDropped();


    int getIfIndex();

private:
    typedef enum {
        ts_mode_SIOCSHWTSTAMP,
        ts_mode_SIOCGSTAMPNS,
        ts_mode_SIOCGSTAMP
    } ts_mode_t;

    int _idx;
    bool _isOpen;
    QString _name;

    MeasurementInterface _settings;

    can_config_t _config;
    can_status_t _status;
    ts_mode_t _ts_mode;

    qint64 _heartbeat_time_ms;
    QUdpSocket* _socket;
    const char *cname();

};


/* Duplicated from Linux can.h for compilation on Windows. Not a great solution. */


/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/*
 * Controller Area Network Identifier structure
 *
 * bit 0-28	: CAN identifier (11/29 bit)
 * bit 29	: error message frame flag (0 = data frame, 1 = error message)
 * bit 30	: remote transmission request flag (1 = rtr frame)
 * bit 31	: frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef uint32_t canid_t;

#define CAN_SFF_ID_BITS		11
#define CAN_EFF_ID_BITS		29

/*
 * Controller Area Network Error Message Frame Mask structure
 *
 * bit 0-28	: error class mask (see include/uapi/linux/can/error.h)
 * bit 29-31	: set to zero
 */
typedef uint32_t can_err_mask_t;

/* CAN payload length and DLC definitions according to ISO 11898-1 */
#define CAN_MAX_DLC 8
#define CAN_MAX_RAW_DLC 15
#define CAN_MAX_DLEN 8

/* CAN FD payload length and DLC definitions according to ISO 11898-7 */
#define CANFD_MAX_DLC 15
#define CANFD_MAX_DLEN 64

/**
 * struct can_frame - Classical CAN frame structure (aka CAN 2.0B)
 * @can_id:   CAN ID of the frame and CAN_*_FLAG flags, see canid_t definition
 * @len:      CAN frame payload length in byte (0 .. 8)
 * @can_dlc:  deprecated name for CAN frame payload length in byte (0 .. 8)
 * @__pad:    padding
 * @__res0:   reserved / padding
 * @len8_dlc: optional DLC value (9 .. 15) at 8 byte payload length
 *            len8_dlc contains values from 9 .. 15 when the payload length is
 *            8 bytes but the DLC value (see ISO 11898-1) is greater then 8.
 *            CAN_CTRLMODE_CC_LEN8_DLC flag has to be enabled in CAN driver.
 * @data:     CAN frame payload (up to 8 byte)
 */
struct can_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    union {
        /* CAN frame payload length in byte (0 .. CAN_MAX_DLEN)
         * was previously named can_dlc so we need to carry that
         * name for legacy support
         */
        uint8_t len;
        uint8_t can_dlc; /* deprecated */
    } __attribute__((packed)); /* disable padding added in some ABIs */
    uint8_t __pad; /* padding */
    uint8_t __res0; /* reserved / padding */
    uint8_t len8_dlc; /* optional DLC for 8 byte payload length (9 .. 15) */
    uint8_t data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};

/*
 * defined bits for canfd_frame.flags
 *
 * The use of struct canfd_frame implies the FD Frame (FDF) bit to
 * be set in the CAN frame bitstream on the wire. The FDF bit switch turns
 * the CAN controllers bitstream processor into the CAN FD mode which creates
 * two new options within the CAN FD frame specification:
 *
 * Bit Rate Switch - to indicate a second bitrate is/was used for the payload
 * Error State Indicator - represents the error state of the transmitting node
 *
 * As the CANFD_ESI bit is internally generated by the transmitting CAN
 * controller only the CANFD_BRS bit is relevant for real CAN controllers when
 * building a CAN FD frame for transmission. Setting the CANFD_ESI bit can make
 * sense for virtual CAN interfaces to test applications with echoed frames.
 *
 * The struct can_frame and struct canfd_frame intentionally share the same
 * layout to be able to write CAN frame content into a CAN FD frame structure.
 * When this is done the former differentiation via CAN_MTU / CANFD_MTU gets
 * lost. CANFD_FDF allows programmers to mark CAN FD frames in the case of
 * using struct canfd_frame for mixed CAN / CAN FD content (dual use).
 * N.B. the Kernel APIs do NOT provide mixed CAN / CAN FD content inside of
 * struct canfd_frame therefore the CANFD_FDF flag is disregarded by Linux.
 */
#define CANFD_BRS 0x01 /* bit rate switch (second bitrate for payload data) */
#define CANFD_ESI 0x02 /* error state indicator of the transmitting node */
#define CANFD_FDF 0x04 /* mark CAN FD for dual use of struct canfd_frame */

/**
 * struct canfd_frame - CAN flexible data rate frame structure
 * @can_id: CAN ID of the frame and CAN_*_FLAG flags, see canid_t definition
 * @len:    frame payload length in byte (0 .. CANFD_MAX_DLEN)
 * @flags:  additional flags for CAN FD
 * @__res0: reserved / padding
 * @__res1: reserved / padding
 * @data:   CAN FD frame payload (up to CANFD_MAX_DLEN byte)
 */
struct canfd_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    uint8_t    len;     /* frame payload length in byte */
    uint8_t    flags;   /* additional flags for CAN FD */
    uint8_t    __res0;  /* reserved / padding */
    uint8_t    __res1;  /* reserved / padding */
    uint8_t    data[CANFD_MAX_DLEN] __attribute__((aligned(8)));
};

#define CAN_MTU		(sizeof(struct can_frame))
#define CANFD_MTU	(sizeof(struct canfd_frame))
