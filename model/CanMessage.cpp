/*
 * CanMessage.cpp
 *
 *  Created on: 20.12.2015
 *      Author: hd
 */

#include "CanMessage.h"
#include <endian.h>

enum {
	id_flag_extended = 0x80000000,
	id_flag_rtr      = 0x40000000,
	id_flag_error    = 0x20000000,
	id_mask_extended = 0x1FFFFFFF,
	id_mask_standard = 0x7FF
};

CanMessage::CanMessage()
  : _raw_id(0), _dlc(0), _u64(0)
{
}

CanMessage::CanMessage(uint32_t can_id)
  : _dlc(0), _u64(0)
{
    setId(can_id);
}

CanMessage::CanMessage(const CanMessage &msg)
  : _raw_id(msg._raw_id), _dlc(msg._dlc), _u64(msg._u64), _interface(msg._interface), _timestamp(msg._timestamp)
{
}


uint32_t CanMessage::getRawId() const {
	return _raw_id;
}

void CanMessage::setRawId(const uint32_t raw_id) {
	_raw_id = raw_id;
}

uint32_t CanMessage::getId() const {
	if (isExtended()) {
		return _raw_id & id_mask_extended;
	} else {
		return _raw_id & id_mask_standard;
	}
}

void CanMessage::setId(const uint32_t id) {
	_raw_id &= ~ id_mask_extended;
	_raw_id |= id;
	if (id>0x7FF) {
		setExtended(true);
	}
}

bool CanMessage::isExtended() const {
	return (_raw_id & id_flag_extended) != 0;
}

void CanMessage::setExtended(const bool isExtended) {
	if (isExtended) {
		_raw_id |= id_flag_extended;
	} else {
		_raw_id &= ~id_flag_extended;
	}
}

bool CanMessage::isRTR() const {
	return (_raw_id & id_flag_rtr) != 0;
}

void CanMessage::setRTR(const bool isRTR) {
	if (isRTR) {
		_raw_id |= id_flag_rtr;
	} else {
		_raw_id &= ~id_flag_rtr;
	}
}

bool CanMessage::isErrorFrame() const {
	return (_raw_id & id_flag_error) != 0;
}

void CanMessage::setErrorFrame(const bool isErrorFrame) {
	if (isErrorFrame) {
		_raw_id |= id_flag_error;
	} else {
		_raw_id &= ~id_flag_error;
    }
}

CanInterface *CanMessage::getInterface() const
{
    return _interface;
}

void CanMessage::setInterface(CanInterface *interface)
{
    _interface = interface;
}

uint8_t CanMessage::getLength() const {
	return _dlc;
}

void CanMessage::setLength(const uint8_t dlc) {
	if (dlc<=8) {
		_dlc = dlc;
	} else {
		_dlc = 8;
	}
}

uint8_t CanMessage::getByte(const uint8_t index) const {
	if (index<sizeof(_u8)) {
		return _u8[index];
	} else {
		return 0;
	}
}

void CanMessage::setByte(const uint8_t index, const uint8_t value) {
	if (index<sizeof(_u8)) {
		_u8[index] = value;
    }
}

uint64_t CanMessage::getU64() const
{
    return _u64;
}

uint64_t CanMessage::extractSignal(const uint8_t start_bit, const uint8_t length, const bool isBigEndian) const
{
    uint64_t result = isBigEndian ? be64toh(_u64) : le64toh(_u64);
    result >>= start_bit;
    uint64_t mask = 0xFFFFFFFFFFFFFFFF;
    mask <<= length;
    return result & ~mask;
}

void CanMessage::setData(const uint8_t d0) {
	_dlc = 1;
	_u8[0] = d0;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1) {
	_dlc = 2;
	_u8[0] = d0;
	_u8[1] = d1;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2) {
	_dlc = 3;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3) {
	_dlc = 4;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4) {
	_dlc = 5;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5) {
	_dlc = 6;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5,
		const uint8_t d6) {
	_dlc = 7;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
	_u8[6] = d6;
}

void CanMessage::setData(const uint8_t d0, const uint8_t d1, const uint8_t d2,
		const uint8_t d3, const uint8_t d4, const uint8_t d5, const uint8_t d6,
		const uint8_t d7) {
	_dlc = 8;
	_u8[0] = d0;
	_u8[1] = d1;
	_u8[2] = d2;
	_u8[3] = d3;
	_u8[4] = d4;
	_u8[5] = d5;
	_u8[6] = d6;
    _u8[7] = d7;
}

timeval CanMessage::getTimestamp() const
{
    return _timestamp;
}

void CanMessage::setTimestamp(const timeval timestamp)
{
    _timestamp = timestamp;
}

QString CanMessage::getIdString() const
{
    if (isExtended()) {
        return QString().sprintf("0x%08X", getId());
    } else {
        return QString().sprintf("0x%03X", getId());
    }
}

QString CanMessage::getDataHexString() const
{
    switch (getLength()) {
        case 0: return "";
        case 1: return QString().sprintf("%02X", getByte(0));
        case 2: return QString().sprintf("%02X %02X", getByte(0), getByte(1));
        case 3: return QString().sprintf("%02X %02X %02X", getByte(0), getByte(1), getByte(2));
        case 4: return QString().sprintf("%02X %02X %02X %02X", getByte(0), getByte(1), getByte(2), getByte(3));
        case 5: return QString().sprintf("%02X %02X %02X %02X %02X", getByte(0), getByte(1), getByte(2), getByte(3), getByte(4));
        case 6: return QString().sprintf("%02X %02X %02X %02X %02X %02X", getByte(0), getByte(1), getByte(2), getByte(3), getByte(4), getByte(5));
        case 7: return QString().sprintf("%02X %02X %02X %02X %02X %02X %02X", getByte(0), getByte(1), getByte(2), getByte(3), getByte(4), getByte(5), getByte(6));
        case 8: return QString().sprintf("%02X %02X %02X %02X %02X %02X %02X %02X", getByte(0), getByte(1), getByte(2), getByte(3), getByte(4), getByte(5), getByte(6), getByte(7));
        default: return QString();
    }

}
