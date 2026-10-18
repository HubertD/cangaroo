#include "UdsDecoder.h"

UdsDecoder::UdsDecoder() {}

DecodeStatus UdsDecoder::tryDecode(const CanMessage& frame, ProtocolMessage& outMsg) {
    if (frame.isErrorFrame() || frame.isRTR() || frame.getLength() < 1) {
        return DecodeStatus::Ignored;
    }

    uint8_t type = (frame.getByte(0) >> 4) & 0x0F;
    uint32_t id = frame.getId();

    if (frame.isExtended()) {
        uint8_t pf = (id >> 16) & 0xFF;
        if (pf == 0xEC || pf == 0xEB) { // J1939 TP.CM or TP.DT
            return DecodeStatus::Ignored;
        }
    }

    if (type == 0) { // Single Frame
        int size = frame.getByte(0) & 0x0F;
        if (size > 0 && size <= frame.getLength() - 1) {
            uint8_t sid = frame.getByte(1);
            
            // Strict check: Only consider UDS if the SID is a recognized service
            // to avoid capturing generic 11-bit traffic (like cangen -L 8).
            QString serviceName = interpretService(sid);
            if (serviceName.startsWith("sid 0x")) {
                // Not a recognized service, probably not UDS
                return DecodeStatus::Ignored;
            }

            outMsg.payload = QByteArray();
            for (int i = 0; i < size; ++i) {
                outMsg.payload.append(frame.getByte(i + 1));
            }
            outMsg.rawFrames = {frame};
            outMsg.protocol = "uds";
            outMsg.timestamp = static_cast<uint64_t>(frame.getFloatTimestamp() * 1000000.0);
            
            outMsg.id = sid;
            outMsg.name = serviceName;
            
            // Type detection: Request vs Response
            // In Cangaroo, we can often tell by ID (e.g., 0x7E0 is req, 0x7E8 is resp)
            // But generically: sid & 0x40 is response, unless it's 0x7F.
            if (sid == 0x7F) {
                outMsg.type = MessageType::NegativeResponse;
                if (outMsg.payload.size() >= 3) {
                    uint8_t nrc = outMsg.payload.at(2);
                    outMsg.description = QString("negative response: %1").arg(interpretNrc(nrc));
                } else {
                    outMsg.description = "negative response: incomplete";
                }
            } else if (sid & 0x40) {
                outMsg.type = MessageType::PositiveResponse;
                outMsg.description = QString("positive response to 0x%1").arg(sid - 0x40, 2, 16, QChar('0'));
            } else {
                outMsg.type = MessageType::Request;
                outMsg.description = QString("uds request, sid 0x%1").arg(sid, 2, 16, QChar('0'));
            }
            
            m_sessions.remove(id); // Clear any pending multi-frame on this ID
            return DecodeStatus::Completed;
        }
    } else if (type == 1) { // First Frame
        int size = ((frame.getByte(0) & 0x0F) << 8) | frame.getByte(1);
        IsotpSession& session = m_sessions[id];
        session.data = QByteArray();
        for (int i = 2; i < frame.getLength(); ++i) {
            session.data.append(frame.getByte(i));
        }
        session.expectedSize = size;
        session.nextSn = 1;
        session.frames = {frame};
        session.rxId = id;
        return DecodeStatus::Consumed;
    } else if (type == 2) { // Consecutive Frame
        if (m_sessions.contains(id)) {
            IsotpSession& session = m_sessions[id];
            uint8_t sn = frame.getByte(0) & 0x0F;
            if (sn == session.nextSn) {
                session.frames.append(frame);
                for (int i = 1; i < frame.getLength() && session.data.size() < session.expectedSize; ++i) {
                    session.data.append(frame.getByte(i));
                }
                session.nextSn = (session.nextSn + 1) % 16;

                if (session.data.size() >= session.expectedSize) {
                    outMsg.payload = session.data;
                    outMsg.rawFrames = session.frames;
                    outMsg.protocol = "uds";
                    outMsg.timestamp = static_cast<uint64_t>(session.frames.first().getFloatTimestamp() * 1000000.0);
                    
                    uint8_t sid = outMsg.payload.at(0);
                    outMsg.id = sid;
                    outMsg.name = interpretService(sid);

                    if (sid == 0x7F) {
                        outMsg.type = MessageType::NegativeResponse;
                        if (outMsg.payload.size() >= 3) {
                            outMsg.description = QString("negative response: %1").arg(interpretNrc(outMsg.payload.at(2)));
                        } else {
                            outMsg.description = "negative response: incomplete";
                        }
                    } else if (sid & 0x40) {
                        outMsg.type = MessageType::PositiveResponse;
                        outMsg.description = QString("positive response to 0x%1").arg(sid - 0x40, 2, 16, QChar('0'));
                    } else {
                        outMsg.type = MessageType::Request;
                        outMsg.description = QString("uds request, sid 0x%1").arg(sid, 2, 16, QChar('0'));
                    }
                    
                    m_sessions.remove(id);
                    return DecodeStatus::Completed;
                }
                return DecodeStatus::Consumed;
            } else {
                // Out of sequence or interleaved? For now just reset.
                m_sessions.remove(id);
                return DecodeStatus::Ignored;
            }
        }
    } else if (type == 3) {
        // Flow Control - ignore for passive decoding, but technically it is "Consumed" for the session
        return DecodeStatus::Consumed;
    }

    return DecodeStatus::Ignored;
}

void UdsDecoder::reset() {
    m_sessions.clear();
}

QString UdsDecoder::interpretService(uint8_t sid) {
    if (sid == 0x7F) return "NegativeResponse";

    bool isResponse = (sid & 0x40);
    uint8_t baseSid = isResponse ? (sid - 0x40) : sid;

    switch (baseSid) {
        case 0x10: return "DiagnosticSessionControl";
        case 0x11: return "EcuReset";
        case 0x14: return "ClearDiagnosticInformation";
        case 0x19: return "ReadDTCInformation";
        case 0x22: return "ReadDataByIdentifier";
        case 0x23: return "ReadMemoryByAddress";
        case 0x27: return "SecurityAccess";
        case 0x28: return "CommunicationControl";
        case 0x2E: return "WriteDataByIdentifier";
        case 0x2F: return "InputOutputControlByIdentifier";
        case 0x31: return "RoutineControl";
        case 0x34: return "RequestDownload";
        case 0x35: return "RequestUpload";
        case 0x36: return "TransferData";
        case 0x37: return "RequestTransferExit";
        case 0x3E: return "TesterPresent";
        case 0x85: return "ControlDTCSetting";
        default: return QString("sid 0x%1").arg(sid, 2, 16, QChar('0'));
    }
}

QString UdsDecoder::interpretNrc(uint8_t nrc) {
    switch (nrc) {
        case 0x10: return "General Reject";
        case 0x11: return "Service Not Supported";
        case 0x12: return "SubFunction Not Supported";
        case 0x13: return "Incorrect Message Length Or Invalid Format";
        case 0x14: return "Response Too Tall";
        case 0x21: return "Busy Repeat Request";
        case 0x22: return "Conditions Not Correct";
        case 0x24: return "Request Sequence Error";
        case 0x25: return "No Response From Subnet Component";
        case 0x26: return "Failure Prevents Execution Of Requested Action";
        case 0x31: return "Request Out Of Range";
        case 0x33: return "Security Access Denied";
        case 0x35: return "Invalid Key";
        case 0x36: return "Exceed Number Of Attempts";
        case 0x37: return "Required Time Delay Not Expired";
        case 0x70: return "Upload Download Not Accepted";
        case 0x71: return "Transfer Data Suspended";
        case 0x72: return "General Programming Failure";
        case 0x73: return "Wrong Block Sequence Counter";
        case 0x78: return "Request Correctly Received-Response Pending";
        case 0x7E: return "SubFunction Not Supported In Active Session";
        case 0x7F: return "Service Not Supported In Active Session";
        case 0x81: return "Rpm Too High";
        case 0x82: return "Rpm Too Low";
        case 0x83: return "Engine Is Running";
        case 0x84: return "Engine Is Not Running";
        case 0x85: return "Engine Run Time Too Low";
        case 0x86: return "Temperature Too High";
        case 0x87: return "Temperature Too Low";
        case 0x88: return "Vehicle Speed Too High";
        case 0x89: return "Vehicle Speed Too Low";
        default: return QString("unknown (0x%1)").arg(nrc, 2, 16, QChar('0'));
    }
}
