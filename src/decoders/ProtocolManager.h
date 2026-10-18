#pragma once

#include "IDecoder.h"
#include <QVector>
#include <memory>

struct ProtocolConfig {
    bool enableUds29Bit = false;
};

class ProtocolManager {
public:
    ProtocolManager();
    ~ProtocolManager() = default;

    /**
     * @brief Processes a CAN frame through all registered decoders.
     * @return The status of the decoding process.
     */
    DecodeStatus processFrame(const CanMessage& frame, ProtocolMessage& outMsg);

    void reset();
    
    ProtocolConfig& config() { return m_config; }

private:
    std::shared_ptr<IDecoder> m_udsDecoder;
    std::shared_ptr<IDecoder> m_j1939Decoder;
    ProtocolConfig m_config;
    uint32_t m_msgCounter = 0;
};
