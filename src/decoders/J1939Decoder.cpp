#include "J1939Decoder.h"

J1939Decoder::J1939Decoder() {}

DecodeStatus J1939Decoder::tryDecode(const CanMessage& frame, ProtocolMessage& outMsg) {
    if (frame.isErrorFrame() || frame.isRTR() || !frame.isExtended()) {
        return DecodeStatus::Ignored;
    }

    uint32_t id = frame.getId();
    uint32_t pgn = extractPgn(id);
    uint8_t sa = id & 0xFF;

    if (pgn == 0x00EC00) { // TP.CM
        uint8_t controlByte = frame.getByte(0);
        if (controlByte == 32 || controlByte == 16) { // BAM (32) or RTS (16)
            uint16_t size = frame.getByte(1) | (frame.getByte(2) << 8);
            uint8_t totalPackets = frame.getByte(3);
            uint32_t targetPgn = frame.getByte(5) | (frame.getByte(6) << 8) | (frame.getByte(7) << 16);
            
            uint32_t sessionKey = sa; // Simplification: track by Source Address
            J1939Session& session = m_sessions[sessionKey];
            session.pgn = targetPgn;
            session.expectedSize = size;
            session.expectedPackets = totalPackets;
            session.receivedPackets = 0;
            session.data = QByteArray();
            session.frames = {frame};
            session.sa = sa;
            return DecodeStatus::Consumed;
        }
    } else if (pgn == 0x00EB00) { // TP.DT
        uint32_t sessionKey = sa;
        if (m_sessions.contains(sessionKey)) {
            J1939Session& session = m_sessions[sessionKey];
            session.receivedPackets++;
            session.frames.append(frame);
            for (int i = 1; i < 8 && session.data.size() < session.expectedSize; ++i) {
                session.data.append(frame.getByte(i));
            }

            if (session.receivedPackets >= session.expectedPackets || session.data.size() >= session.expectedSize) {
                outMsg.payload = session.data;
                outMsg.rawFrames = session.frames;
                outMsg.protocol = "J1939";
                outMsg.timestamp = static_cast<uint64_t>(session.frames.first().getFloatTimestamp() * 1000000.0);
                outMsg.id = session.pgn;
                outMsg.type = MessageType::Request; // Default for J1939
                
                if (session.pgn == 0xFEEC) outMsg.name = "Vehicle Identification (VIN)";
                else if (session.pgn == 0xF004) outMsg.name = "Electronic Engine Controller 1 (EEC1)";
                else outMsg.name = QString("PGN: 0x%2")
                  .arg(session.pgn, 4, 16, QChar('0').toUpper());
                
                // Populate metadata from the first frame (CM header)
                uint32_t firstId = session.frames.first().getId();
                outMsg.metadata["Priority"] = (firstId >> 26) & 0x7;
                outMsg.metadata["Reserved"] = (firstId >> 25) & 1;
                outMsg.metadata["Data Page"] = (firstId >> 24) & 1;
                outMsg.metadata["PDU Format"] = (firstId >> 16) & 0xFF;
                outMsg.metadata["PDU Specific"] = (firstId >> 8) & 0xFF;
                outMsg.metadata["Source Address"] = firstId & 0xFF;

                m_sessions.remove(sessionKey);
                return DecodeStatus::Completed;
            }
            return DecodeStatus::Consumed;
        }
    } else {
        // Single frame PGN
        uint8_t pf = (id >> 16) & 0xFF;
        uint8_t ps = (id >> 8) & 0xFF;
        bool isPDU1 = pf < 240;

        outMsg.payload = QByteArray();
        for (int i = 0; i < frame.getLength(); ++i) {
            outMsg.payload.append(frame.getByte(i));
        }
        outMsg.rawFrames = {frame};
        outMsg.protocol = "J1939";
        outMsg.timestamp = static_cast<uint64_t>(frame.getFloatTimestamp() * 1000000.0);
        outMsg.id = pgn;
        outMsg.type = MessageType::Request;
        
        if (pgn == 0xFEEC) outMsg.name = "Vehicle Identification (VIN)";
        else if (pgn == 0xF004) outMsg.name = "Electronic Engine Controller 1 (EEC1)";
        else outMsg.name = QString("PGN: 0x%2")
                  .arg(pgn, 4, 16, QChar('0').toUpper());
        
        if (isPDU1) {
            outMsg.description = QString("PDU1 (Peer-to-Peer) from SA [0x%1] to DA [0x%2]").arg(sa, 2, 16, QChar('0').toUpper()).arg(ps, 2, 16, QChar('0').toUpper());
        } else {
            outMsg.description = QString("PDU2 (Broadcast) from SA [0x%1]").arg(sa, 2, 16, QChar('0').toUpper());
        }

        outMsg.metadata["Priority"] = (id >> 26) & 0x7;
        outMsg.metadata["Reserved"] = (id >> 25) & 1;
        outMsg.metadata["Data Page"] = (id >> 24) & 1;
        outMsg.metadata["PDU Format"] = pf;
        outMsg.metadata["PDU Specific"] = ps;
        outMsg.metadata["Source Address"] = sa;

        return DecodeStatus::Completed;
    }

    return DecodeStatus::Ignored;
}

void J1939Decoder::reset() {
    m_sessions.clear();
}

uint32_t J1939Decoder::extractPgn(uint32_t id) {
    uint8_t pf = (id >> 16) & 0xFF;
    uint8_t ps = (id >> 8) & 0xFF;
    uint8_t dp = (id >> 24) & 1;
    uint8_t edp = (id >> 25) & 1;

    uint32_t pgn = (edp << 17) | (dp << 16) | (pf << 8);
    if (pf >= 240) {
        pgn |= ps;
    }
    return pgn;
}
