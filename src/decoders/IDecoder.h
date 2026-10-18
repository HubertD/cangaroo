#pragma once

#include "ProtocolTypes.h"

enum class DecodeStatus {
    Ignored,   ///< Frame not relevant to this decoder
    Consumed,  ///< Frame accepted, sequence in progress
    Completed  ///< Frame accepted, sequence finished, outMsg populated
};

class IDecoder {
public:
    virtual ~IDecoder() = default;

    /**
     * @brief Tries to decode an incoming CAN frame.
     * @param frame The raw CAN frame.
     * @param outMsg If a full protocol message is reassembled/decoded, it is stored here.
     * @return The status of the decoding process.
     */
    virtual DecodeStatus tryDecode(const CanMessage& frame, ProtocolMessage& outMsg) = 0;

    /**
     * @brief Resets the decoder state (e.g. for multiple channels or starting trace).
     */
    virtual void reset() = 0;
};
