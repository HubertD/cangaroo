#ifndef GRIPHANDLER_H
#define GRIPHANDLER_H


#include "GrIP.h"
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <atomic>
#include <memory>
#include <cstdint>
#include "core/CanMessage.h"


typedef struct
{
    GrIP_ProtocolType_e ProtType;
    GrIP_MessageType_e MsgType;
    GrIP_ReturnType_e ReturnCode;
    GrIP_Pdu_t PDU;
} SendPacket_t;


class GrIPHandler
{
public:
    GrIPHandler(const QString &name);
    ~GrIPHandler();

    GrIPHandler(const GrIPHandler&) = delete;
    GrIPHandler& operator=(const GrIPHandler&) = delete;

    bool Start();
    void Stop();

    void SetStatus(bool open);
    void SetEchoTx(bool enable);

    void RequestVersion();
    std::string GetVersion() const;

    int Channels_CAN() const;
    int Channels_CANFD() const;

    void EnableChannel(uint8_t ch, bool enable);
    void Mode(uint8_t ch, bool listen_only);

    void CAN_SetBaudrate(uint8_t ch, uint32_t baud);

    bool CanAvailable(uint8_t ch) const;
    CanMessage ReceiveCan(uint8_t ch);

    bool CanTransmit(uint8_t ch, const CanMessage &msg);


    void Send(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const GrIP_Pdu_t *pdu);
    void Send(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const uint8_t *data, uint16_t len);

private:
    void ProcessData(GrIP_Packet_t &packet);
    void WorkerThread();

    QSerialPort *m_SerialPort;
    mutable std::mutex m_MutexSerial;
    mutable std::mutex m_MutexData;

    std::unique_ptr<std::thread> m_pWorkerThread;
    mutable std::mutex m_MutexCanQueue;
    //std::queue<SendPacket_t> m_SendQueue;
    std::vector<std::queue<CanMessage>> m_ReceiveQueue;
    //std::queue<CanMessage> m_ReceiveQueue1;
    //std::queue<CanMessage> m_ReceiveQueue2;

    std::atomic<bool> m_Exit;

    std::string m_Version;
    int m_ChannelsCAN;
    int m_ChannelsCANFD;

    std::vector<bool> m_Channel_StatusCAN;

};


#endif // GRIPHANDLER_H
