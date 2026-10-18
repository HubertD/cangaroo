#include "ProtocolManager.h"
#include "UdsDecoder.h"
#include "J1939Decoder.h"

ProtocolManager::ProtocolManager() {
    m_udsDecoder = std::make_shared<UdsDecoder>();
    m_j1939Decoder = std::make_shared<J1939Decoder>();
}

DecodeStatus ProtocolManager::processFrame(const CanMessage& frame, ProtocolMessage& outMsg) {
    DecodeStatus status = DecodeStatus::Ignored;

    if (frame.isExtended()) {
        // 29-bit ID: Try J1939 first
        status = m_j1939Decoder->tryDecode(frame, outMsg);
        
        // Only try UDS if J1939 ignored it and 29-bit UDS is explicitly enabled
        if (status == DecodeStatus::Ignored && m_config.enableUds29Bit) {
            status = m_udsDecoder->tryDecode(frame, outMsg);
        }
    } else {
        // 11-bit ID: Try UDS
        status = m_udsDecoder->tryDecode(frame, outMsg);
    }

    if (status == DecodeStatus::Completed) {
        outMsg.globalIndex = ++m_msgCounter;
    }

    return status;
}

void ProtocolManager::reset() {
    m_msgCounter = 0;
    m_udsDecoder->reset();
    m_j1939Decoder->reset();
}
