/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

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



#include "CanMessage.h"
#include <core/portable_endian.h>

// J1939 / many ECUs place multi-byte Intel (@1+) values with MSB in the lower
// byte index on the bus (e.g. 100 km/h → bytes 6=0x64, 7=0x00). DBC bit-LSE
// assembly yields the swapped numeric value; fix up whole-byte aligned fields.
static bool intelUsesSwappedBytes(uint8_t bit_shift, uint8_t length)
{
    return bit_shift == 0 && (length == 16 || length == 32 || length == 64);
}

static uint64_t swapIntelRawBytes(uint64_t value, uint8_t length)
{
    switch (length) {
    case 16:
        return ((value & 0xFF) << 8) | ((value >> 8) & 0xFF);
    case 32:
        return ((value & 0xFF) << 24) | ((value & 0xFF00) << 8)
             | ((value & 0xFF0000) >> 8) | ((value >> 24) & 0xFF);
    case 64:
        return ((value & 0xFF) << 56) | ((value & 0xFF00ULL) << 40)
             | ((value & 0xFF0000ULL) << 24) | ((value & 0xFF000000ULL) << 8)
             | ((value & 0xFF00000000ULL) >> 8) | ((value & 0xFF0000000000ULL) >> 24)
             | ((value & 0xFF000000000000ULL) >> 40) | ((value >> 56) & 0xFF);
    default:
        return value;
    }
}

enum {
	id_flag_extended = 0x80000000,
	id_flag_rtr      = 0x40000000,
	id_flag_error    = 0x20000000,
	id_mask_extended = 0x1FFFFFFF,
	id_mask_standard = 0x7FF
};

CanMessage::CanMessage()
    : _raw_id(0), _dlc(0), _isFD(false), _isBRS(false), _isRX(true), _isShow(true), _interface(0), _u8()
{
    _timestamp.tv_sec = 0;
    _timestamp.tv_usec = 0;

}

CanMessage::CanMessage(uint32_t can_id)
    : _dlc(0), _isFD(false), _isBRS(false), _isRX(true), _isShow(true), _interface(0), _u8()
{
    _timestamp.tv_sec = 0;
    _timestamp.tv_usec = 0;
    setId(can_id);
}

CanMessage::CanMessage(const CanMessage &msg)
{
    cloneFrom(msg);
}

void CanMessage::cloneFrom(const CanMessage &msg)
{
    _raw_id = msg._raw_id;
    _dlc = msg._dlc;
    _isFD = msg._isFD;
    _isBRS = msg._isBRS;
    _isRX = msg._isRX;
    _isShow = msg._isShow;

    // Copy data
    for(int i=0; i<64; i++)
    {
        _u8[i] = msg._u8[i];
    }

    _interface = msg._interface;
    _timestamp = msg._timestamp;
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

uint32_t CanMessage::j1939Pgn(uint32_t id) {
	uint8_t pf  = (id >> 16) & 0xFF;
	uint8_t ps  = (id >>  8) & 0xFF;
	uint8_t dp  = (id >> 24) & 1;
	uint8_t edp = (id >> 25) & 1;

	uint32_t pgn = (edp << 17) | (dp << 16) | (pf << 8);
	if (pf >= 240) { // PDU2 (broadcast): PS is part of the PGN
		pgn |= ps;
	}
	return pgn;
}

void CanMessage::setId(const uint32_t id) {
    _raw_id &= ~ id_mask_extended;
    _raw_id = id;
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

bool CanMessage::isFD() const {
    return _isFD;
}

void CanMessage::setFD(const bool isFD) {
    _isFD = isFD;
}

bool CanMessage::isBRS() const {
    return _isBRS;
}

void CanMessage::setBRS(const bool isBRS) {
    _isBRS = isBRS;
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

CanInterfaceId CanMessage::getInterfaceId() const
{
    return _interface;
}

void CanMessage::setInterfaceId(CanInterfaceId interface)
{
    _interface = interface;
}

uint8_t CanMessage::getLength() const {
	return _dlc;
}

void CanMessage::setLength(const uint8_t dlc) {
    // Limit to CANFD max length
    if (dlc<=64) {
		_dlc = dlc;
	} else {
		_dlc = 8;
	}
}

bool CanMessage::isRX() const {
    return _isRX;
}

void CanMessage::setRX(const bool isRX) {
    _isRX = isRX;
}

bool CanMessage::isShow() const {
    return _isShow;
}

void CanMessage::setShow(const bool enable) {
    _isShow = enable;
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

uint64_t CanMessage::extractRawSignal(uint8_t start_bit, const uint8_t length, const bool isBigEndian) const
{
    if (length == 0 || start_bit >= sizeof(_u8) * 8 || length > 64) return 0;

    if (!isBigEndian) {
        // Intel (@1+): little-endian byte assembly + bit shift (pre-044201d path)
        const int byte_offset = start_bit / 8;
        const int bit_shift   = start_bit % 8;

        uint8_t temp[8] = {0};
        int copy_len = static_cast<int>(sizeof(_u8)) - byte_offset;
        if (copy_len > 8) {
            copy_len = 8;
        }

        for (int i = 0; i < copy_len; ++i) {
            temp[i] = _u8[byte_offset + i];
        }

        uint64_t data_raw = 0;
        for (int i = 0; i < 8; ++i) {
            data_raw |= (static_cast<uint64_t>(temp[i]) << (i * 8));
        }

        uint64_t data = data_raw >> bit_shift;

        uint64_t mask = 0xFFFFFFFFFFFFFFFFULL;
        if (length < 64) {
            mask = (1ULL << length) - 1;
        }
        data &= mask;
        if (intelUsesSwappedBytes(bit_shift, length)) {
            data = swapIntelRawBytes(data, length);
        }
        return data;
    } else {
        uint64_t data = 0;
        int currentBit = start_bit;
        for (int i = 0; i < length; ++i) {
            int byteIndex = currentBit / 8;
            int bitInByte = currentBit % 8;
            
            if (byteIndex < sizeof(_u8)) {
                if (_u8[byteIndex] & (1 << bitInByte)) {
                    data |= (1ULL << (length - 1 - i));
                }
            }
            
            if ((currentBit % 8) == 0) {
                currentBit += 15;
            } else {
                currentBit--;
            }
        }
        return data;
    }
}

void CanMessage::setRawSignal(uint8_t start_bit, const uint8_t length, const bool isBigEndian, uint64_t value)
{
    if (length == 0 || start_bit >= sizeof(_u8) * 8 || length > 64) return;

    if (!isBigEndian) {
        // Intel (@1+): inverse of extractRawSignal byte-shift path (a6042cc)
        const int byte_offset = start_bit / 8;
        const int bit_shift   = start_bit % 8;

        uint64_t mask = 0xFFFFFFFFFFFFFFFFULL;
        if (length < 64) {
            mask = (1ULL << length) - 1;
        }

        uint64_t val = value & mask;
        if (intelUsesSwappedBytes(bit_shift, length)) {
            val = swapIntelRawBytes(val, length);
        }
        const uint64_t valToSet   = val << bit_shift;
        const uint64_t clearMask  = ~(mask << bit_shift);

        uint8_t temp[8] = {0};
        int copy_len = static_cast<int>(sizeof(_u8)) - byte_offset;
        if (copy_len > 8) {
            copy_len = 8;
        }

        for (int i = 0; i < copy_len; ++i) {
            temp[i] = _u8[byte_offset + i];
        }

        uint64_t data_raw = 0;
        for (int i = 0; i < 8; ++i) {
            data_raw |= (static_cast<uint64_t>(temp[i]) << (i * 8));
        }

        data_raw &= clearMask;
        data_raw |= valToSet;

        for (int i = 0; i < copy_len; ++i) {
            _u8[byte_offset + i] = static_cast<uint8_t>((data_raw >> (i * 8)) & 0xFF);
        }
    } else {
        uint64_t mask = 0xFFFFFFFFFFFFFFFFULL;
        if (length < 64) {
            mask = (1ULL << length) - 1;
        }
        uint64_t val = value & mask;

        int currentBit = start_bit;
        for (int i = 0; i < length; ++i) {
            int byteIndex = currentBit / 8;
            int bitInByte = currentBit % 8;
            
            if (byteIndex < sizeof(_u8)) {
                if (val & (1ULL << (length - 1 - i))) {
                    _u8[byteIndex] |= (1 << bitInByte);
                } else {
                    _u8[byteIndex] &= ~(1 << bitInByte);
                }
            }
            
            if ((currentBit % 8) == 0) {
                currentBit += 15;
            } else {
                currentBit--;
            }
        }
    }
}

void CanMessage::setDataAt(uint8_t position, uint8_t data)
{
    if(position < 64)
        _u8[position] = data;
    else
        return;
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

void CanMessage::setTimestamp(const uint64_t seconds, const uint32_t micro_seconds)
{
    _timestamp.tv_sec = seconds;
    _timestamp.tv_usec = micro_seconds;
}

double CanMessage::getFloatTimestamp() const
{
    return (double)_timestamp.tv_sec + ((double)_timestamp.tv_usec/1000000);
}

QDateTime CanMessage::getDateTime() const
{
    return QDateTime::fromMSecsSinceEpoch((qint64)(1000*getFloatTimestamp()));
}

QString CanMessage::getIdString() const
{
    if (isExtended()) {
        return QString().asprintf("0x%08X", getId());
    } else {
        return QString().asprintf("0x%03X", getId());
    }
}

QString CanMessage::getDataHexString() const
{
    if(getLength() == 0)
        return "";

    QString outstr = "";
    for(int i=0; i<getLength(); i++)
    {
        outstr += QString().asprintf("%02X ", getByte(i));
    }

    return outstr;
}
