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

#ifndef BASE_CANMESSAGE_H_
#define BASE_CANMESSAGE_H_
#include <stdint.h>
#include <sys/time.h>

#include <QString>
#include <driver/CanDriver.h>

class CanMessage {
public:
	CanMessage();
	CanMessage(uint32_t can_id);
    CanMessage(const CanMessage &msg);
    void cloneFrom(const CanMessage &msg);

	uint32_t getRawId() const;
	void setRawId(const uint32_t raw_id);

	uint32_t getId() const;
	void setId(const uint32_t id);

	bool isExtended() const;
	void setExtended(const bool isExtended);

	bool isRTR() const;
	void setRTR(const bool isRTR);

	bool isErrorFrame() const;
	void setErrorFrame(const bool isErrorFrame);

    CanInterfaceId getInterfaceId() const;
    void setInterfaceId(CanInterfaceId interface);

	uint8_t getLength() const;
	void setLength(const uint8_t dlc);

	uint8_t getByte(const uint8_t index) const;
	void setByte(const uint8_t index, const uint8_t value);

    uint64_t getU64() const;
    uint64_t extractRawSignal(uint8_t start_bit, const uint8_t length, const bool isBigEndian) const;

	void setData(const uint8_t d0);
	void setData(const uint8_t d0, const uint8_t d1);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2, const uint8_t d3);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2, const uint8_t d3, const uint8_t d4);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2, const uint8_t d3, const uint8_t d4, const uint8_t d5);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2, const uint8_t d3, const uint8_t d4, const uint8_t d5, const uint8_t d6);
	void setData(const uint8_t d0, const uint8_t d1, const uint8_t d2, const uint8_t d3, const uint8_t d4, const uint8_t d5, const uint8_t d6, const uint8_t d7);

    struct timeval getTimestamp() const;
    void setTimestamp(const struct timeval timestamp);

    double getFloatTimestamp() const;

    QString getIdString() const;
    QString getDataHexString() const;

private:
	uint32_t _raw_id;
    uint8_t _dlc;
    CanInterfaceId _interface;
    union {
		uint8_t _u8[8];
		uint16_t _u16[4];
		uint32_t _u32[2];
		uint64_t _u64;
	};
    struct timeval _timestamp;

};

#endif /* BASE_CANMESSAGE_H_ */
