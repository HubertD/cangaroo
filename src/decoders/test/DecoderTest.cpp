#include <iostream>
#include <iomanip>
#include <cassert>
#include "decoders/UdsDecoder.h"
#include "decoders/J1939Decoder.h"

void testUdsSingleFrame() {
    UdsDecoder decoder;
    CanMessage msg(0x7E0);
    msg.setLength(8);
    msg.setByte(0, 0x02); // SF, size 2
    msg.setByte(1, 0x10); // SID 0x10
    msg.setByte(2, 0x01); // Param 0x01
    
    ProtocolMessage out;
    DecodeStatus result = decoder.tryDecode(msg, out);
    assert(result == DecodeStatus::Completed);
    assert(out.name == "DiagnosticSessionControl");
    assert(out.protocol == "uds");
    assert(out.id == 0x10);
    assert(out.type == MessageType::Request);
    assert(out.payload.size() == 2);
    assert((uint8_t)out.payload[0] == 0x10);
    std::cout << "testUdsSingleFrame passed" << std::endl;
}

void testUdsMultiFrame() {
    UdsDecoder decoder;
    ProtocolMessage out;

    // FF
    CanMessage ff(0x7E0);
    ff.setLength(8);
    ff.setByte(0, 0x10); // FF
    ff.setByte(1, 0x0A); // Size 10
    ff.setByte(2, 0x22); // SID 0x22
    for(int i=3; i<8; i++) ff.setByte(i, i);
    
    assert(decoder.tryDecode(ff, out) == DecodeStatus::Consumed);

    // CF
    CanMessage cf(0x7E0);
    cf.setLength(8);
    cf.setByte(0, 0x21); // CF, SN 1
    for(int i=1; i<6; i++) cf.setByte(i, 0xA0 + i);
    
    DecodeStatus result = decoder.tryDecode(cf, out);
    assert(result == DecodeStatus::Completed);
    assert(out.name == "ReadDataByIdentifier");
    assert(out.payload.size() == 10);
    assert(out.id == 0x22);
    assert(out.type == MessageType::Request);
    std::cout << "testUdsMultiFrame passed" << std::endl;
}

void testJ1939SingleFrame() {
    J1939Decoder decoder;
    CanMessage msg(0x18FEEF01); // PGN 65263 (EngTemp), SA 1, Pri 6
    msg.setExtended(true);
    msg.setLength(8);
    for(int i=0; i<8; i++) msg.setByte(i, i);

    ProtocolMessage out;
    DecodeStatus result = decoder.tryDecode(msg, out);
    assert(result == DecodeStatus::Completed);
    assert(out.name == "PGN: 0xFF17");
    assert(out.protocol == "J1939");
    assert(out.id == 65263);
    assert(out.type == MessageType::Request);
    std::cout << "testJ1939SingleFrame passed" << std::endl;
}

void testUdsNegativeResponse() {
    UdsDecoder decoder;
    CanMessage msg(0x7E8);
    msg.setLength(8);
    msg.setByte(0, 0x03); // SF
    msg.setByte(1, 0x7F); // SID 0x7F (Negative Response)
    msg.setByte(2, 0x22); // SID being rejected
    msg.setByte(3, 0x33); // NRC 0x33 (Security Access Denied)
    
    ProtocolMessage out;
    DecodeStatus result = decoder.tryDecode(msg, out);
    assert(result == DecodeStatus::Completed);
    assert(out.type == MessageType::NegativeResponse);
    assert(out.name == "NegativeResponse");
    assert(out.description == "negative response: Security Access Denied");
    std::cout << "testUdsNegativeResponse passed" << std::endl;
}

int main() {
    testUdsSingleFrame();
    testUdsMultiFrame();
    testJ1939SingleFrame();
    testUdsNegativeResponse();
    std::cout << "All decoder tests passed!" << std::endl;
    return 0;
}
