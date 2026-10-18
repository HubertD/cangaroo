#include "GrIPHandler.h"
#include "CRC.h"
//#include "Protocol.h"
#include <chrono>
#include <cstring>


#define SYSTEM_REPORT_INFO      0u
#define SYSTEM_SET_STATUS       1u

#define SYSTEM_SEND_CAN_CFG     20u
#define SYSTEM_SEND_LIN_CFG     21u
#define SYSTEM_START_CAN        22u
#define SYSTEM_START_LIN        23u
#define SYSTEM_ADD_CAN_FRAME    24u
#define SYSTEM_ADD_LIN_FRAME    25u
#define SYSTEM_CAN_MODE         26u
#define SYSTEM_CAN_TXECHO       27u
#define SYSTEM_SEND_CAN_FRAME   30u

// CAN Msg flags
#define CAN_FLAGS_EXT_ID            0x01
#define CAN_FLAGS_FD                0x02
#define CAN_FLAGS_RTR               0x04
#define CAN_FLAGS_BRS               0x08
#define CAN_FLAGS_TX                0x80


typedef struct __attribute__((packed))
{
    uint8_t Version;
    uint8_t Command;
    uint8_t Length;
    uint8_t Data;
} Protocol_SystemHeader_t;

typedef struct __attribute__((packed))
{
    Protocol_SystemHeader_t Header;
    uint8_t Channel1;
    uint8_t Channel2;
} Protocol_ChannelStatus_t;

typedef struct __attribute__((packed))
{
    Protocol_SystemHeader_t Header;
    uint8_t Channel;
    uint8_t Mode;
} Protocol_ChannelMode_t;

typedef struct __attribute__((packed))
{
    Protocol_SystemHeader_t Header;

    uint8_t Channel;
    uint32_t ID;
    uint8_t DLC;
    uint8_t Flags;
    uint8_t ErrFlags;
    uint32_t Time;
    uint8_t Data[64];
} Protocol_CanFrame_t;


GrIPHandler::GrIPHandler(const QString &name)
{
    m_Exit = false;
    m_ChannelsCAN = 0;
    m_ChannelsCANFD = 0;

    CRC_Init();

    m_SerialPort = new QSerialPort();
    m_SerialPort->setPortName(name);
    m_SerialPort->setBaudRate(1000000);
    m_SerialPort->setDataBits(QSerialPort::Data8);
    m_SerialPort->setParity(QSerialPort::NoParity);
    m_SerialPort->setStopBits(QSerialPort::OneStop);
    m_SerialPort->setFlowControl(QSerialPort::NoFlowControl);
    m_SerialPort->setReadBufferSize(2048);

    GrIP_Init(*m_SerialPort);
}


GrIPHandler::~GrIPHandler()
{
    Stop();
    delete m_SerialPort;
}


bool GrIPHandler::Start()
{
    std::unique_lock<std::mutex> lck(m_MutexSerial);

    if(!m_SerialPort->open(QIODevice::ReadWrite))
    {
        perror("Serport connect failed!");
        return false;
    }

    m_SerialPort->flush();
    m_SerialPort->clear();

    m_Exit = false;
    m_pWorkerThread = std::make_unique<std::thread>(std::thread(&GrIPHandler::WorkerThread, this));

    return true;
}


void GrIPHandler::Stop()
{
    m_Exit = true;

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    if(m_pWorkerThread->joinable())
    {
        m_pWorkerThread->join();
    }
    m_pWorkerThread.reset();

    if (m_SerialPort->isOpen())
    {
        m_SerialPort->waitForBytesWritten(20);
        m_SerialPort->waitForReadyRead(10);
        m_SerialPort->clear();
        m_SerialPort->close();
    }
}


void GrIPHandler::SetStatus(bool open)
{
    Protocol_SystemHeader_t header;

    header.Version = 1;
    header.Command = SYSTEM_SET_STATUS;
    header.Length = 0;
    header.Data = 1;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&header), sizeof(Protocol_SystemHeader_t)};

    if(open)
    {
        header.Data = 2;
    }

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);
}


void GrIPHandler::SetEchoTx(bool enable)
{
    Protocol_SystemHeader_t header;

    header.Version = 1;
    header.Command = SYSTEM_CAN_TXECHO;
    header.Length = 0;
    header.Data = 0;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&header), sizeof(Protocol_SystemHeader_t)};

    if(enable)
    {
        header.Data = 1;
    }

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);
}


void GrIPHandler::RequestVersion()
{
    Protocol_SystemHeader_t header;

    header.Version = 1;
    header.Command = SYSTEM_REPORT_INFO;
    header.Length = 0;
    header.Data = 0;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&header), sizeof(Protocol_SystemHeader_t)};

    // Clear previous version
    m_Version.clear();

    m_ChannelsCAN = 0;
    m_ChannelsCANFD = 0;

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);
    m_SerialPort->waitForReadyRead(10);
}


std::string GrIPHandler::GetVersion() const
{
    std::unique_lock<std::mutex> lck(m_MutexData);
    return m_Version;
}


int GrIPHandler::Channels_CAN() const
{
    std::unique_lock<std::mutex> lck(m_MutexData);
    return m_ChannelsCAN;
}


int GrIPHandler::Channels_CANFD() const
{
    std::unique_lock<std::mutex> lck(m_MutexData);
    return m_ChannelsCANFD;
}


void GrIPHandler::Send(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const GrIP_Pdu_t *pdu)
{
    std::unique_lock<std::mutex> lck(m_MutexSerial);
    GrIP_Transmit(ProtType, MsgType, ReturnCode, pdu);
}


void GrIPHandler::Send(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const uint8_t *data, uint16_t len)
{
    GrIP_Pdu_t pdu = {(uint8_t*)data, len};
    Send(ProtType, MsgType, ReturnCode, &pdu);
}


void GrIPHandler::EnableChannel(uint8_t ch, bool enable)
{
    Protocol_ChannelStatus_t status = {};

    status.Header.Version = 1;
    status.Header.Command = SYSTEM_START_CAN;
    status.Header.Length = 2;
    status.Header.Data = 0;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&status), sizeof(Protocol_ChannelStatus_t)};

    if(ch == 0)
    {
        status.Channel1 = (uint8_t)enable;
    }
    else if(ch == 1)
    {
        status.Channel2 = (uint8_t)enable;
    }

    if(ch < m_Channel_StatusCAN.size())
    {
        m_Channel_StatusCAN[ch] = enable;
        status.Channel1 = m_Channel_StatusCAN[0];
        status.Channel2 = m_Channel_StatusCAN[1];

        std::unique_lock<std::mutex> lck(m_MutexSerial);

        GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);
    }
}


void GrIPHandler::Mode(uint8_t ch, bool listen_only)
{
    Protocol_ChannelMode_t mode = {};

    mode.Header.Version = 1;
    mode.Header.Command = SYSTEM_CAN_MODE;
    mode.Header.Length = 2;
    mode.Header.Data = 0;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&mode), sizeof(Protocol_ChannelMode_t)};

    mode.Channel = ch;
    mode.Mode = (uint8_t)listen_only;

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);
}


void GrIPHandler::CAN_SetBaudrate(uint8_t ch, uint32_t baud)
{
    std::unique_lock<std::mutex> lck(m_MutexSerial);

    uint8_t msg[9] = {};
    GrIP_Pdu_t p = {msg, 9};

    // Set cmd
    msg[0] = SYSTEM_SEND_CAN_CFG;

    msg[1] = ch;

    msg[2] = (baud >> 24) & 0xFF;
    msg[3] = (baud >> 16) & 0xFF;
    msg[4] = (baud >> 8) & 0xFF;
    msg[5] = (baud) & 0xFF;

    GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p);

    m_SerialPort->waitForBytesWritten(5);
}


bool GrIPHandler::CanAvailable(uint8_t ch) const
{
    std::unique_lock<std::mutex> lck(m_MutexData);

    if(m_ReceiveQueue.size() > ch)
    {
        return (m_ReceiveQueue[ch].size() > 0);
    }

    return false;
}


CanMessage GrIPHandler::ReceiveCan(uint8_t ch)
{
    std::unique_lock<std::mutex> lck(m_MutexData);

    if(m_ReceiveQueue.size() > ch)
    {
        if(m_ReceiveQueue[ch].size())
        {
            auto front = m_ReceiveQueue[ch].front();
            m_ReceiveQueue[ch].pop();
            return front;
        }
    }

    return CanMessage();
}


bool GrIPHandler::CanTransmit(uint8_t ch, const CanMessage &msg)
{
    Protocol_CanFrame_t frame;

    frame.Header.Version = 1;
    frame.Header.Command = SYSTEM_SEND_CAN_FRAME;
    frame.Header.Length = sizeof(Protocol_CanFrame_t) - sizeof(Protocol_SystemHeader_t);
    frame.Header.Data = 0;

    GrIP_Pdu_t p = {reinterpret_cast<uint8_t*>(&frame), sizeof(Protocol_CanFrame_t)};

    uint32_t ID = msg.getId();

    frame.Channel = ch;

    frame.ID = ID;

    frame.DLC = msg.getLength();

    frame.Flags = 0;
    if(msg.isExtended())
    {
        frame.Flags |= CAN_FLAGS_EXT_ID;
    }
    if(msg.isFD())
    {
        frame.Flags |= CAN_FLAGS_FD;
    }
    if(msg.isRTR())
    {
        frame.Flags |= CAN_FLAGS_RTR;
    }

    frame.Time = 0;

    for(int i = 0; i < msg.getLength(); i++)
    {
        frame.Data[i] = msg.getByte(i);
    }

    std::unique_lock<std::mutex> lck(m_MutexSerial);

    return (GrIP_Transmit(PROT_GrIP, MSG_SYSTEM_CMD, RET_OK, &p) == 0);
}


void GrIPHandler::ProcessData(GrIP_Packet_t &packet)
{
    std::unique_lock<std::mutex> lck(m_MutexData);

    switch(packet.RX_Header.MsgType)
    {
    case MSG_SYSTEM_CMD:
        switch(packet.Data[0])
        {
        case 0:
        {
            // System info
            uint8_t major = packet.Data[1];
            uint8_t minor = packet.Data[2];
            //uint8_t hw = packet.Data[3];

            uint8_t can = packet.Data[4];
            uint8_t canfd = packet.Data[5];
            /*uint8_t lin = packet.Data[6];
            uint8_t adc = packet.Data[7];
            uint8_t gpio = packet.Data[8];*/

            char date[128] = {};
            strncpy(date, (char*)&packet.Data[9], 120);

            char buffer[256]{};
            sprintf(buffer, "%d.%d-<%s>", major, minor, date);
            m_Version = buffer;

            m_ChannelsCAN = can;
            m_ChannelsCANFD = canfd;

            m_ReceiveQueue.clear();
            //m_Channel_StatusCAN.clear();

            for(int i = 0; i < can; i++)
            {
                m_Channel_StatusCAN.push_back(false);
                m_ReceiveQueue.push_back({});
            }
            for(int i = 0; i < canfd; i++)
            {
                m_Channel_StatusCAN.push_back(false);
                m_ReceiveQueue.push_back({});
            }

            //fprintf(stderr, "SYS INFO: %s\n", m_Version.c_str());
            break;
        }

        case 1:
            break;

        case 2:
            break;

        default:
            break;
        }
        break;

    case MSG_REALTIME_CMD:
        break;

    case MSG_DATA:
    case MSG_DATA_NO_RESPONSE:
        Protocol_SystemHeader_t header;
        memcpy(&header, packet.Data, sizeof(Protocol_SystemHeader_t));

        switch(header.Command)
        {
        case 254u: // DATA_REPORT_CAN_MSG
        {
            Protocol_CanFrame_t frame;

            memcpy(&frame, packet.Data, sizeof(Protocol_CanFrame_t));

            /*uint8_t ch = packet.Data[1];

            //uint32_t time = packet.Data[2]<<24 | packet.Data[3]<<16 | packet.Data[4]<<8 | packet.Data[5];

            uint32_t id = packet.Data[6]<<24 | packet.Data[7]<<16 | packet.Data[8]<<8 | packet.Data[9];

            uint8_t dlc = packet.Data[10];
            uint8_t flags = packet.Data[11];

            uint8_t data[8] = {};
            memcpy(data, &packet.Data[12], dlc);*/

            qint64 msec = QDateTime::currentMSecsSinceEpoch();

            CanMessage msg(frame.ID);

            // Defaults
            msg.setErrorFrame(false);
            msg.setRTR(false);
            msg.setFD(false);
            msg.setBRS(false);

            msg.setLength(frame.DLC);
            msg.setRX(true);
            msg.setTimestamp({
                static_cast<long>(msec / 1000),        // Seconds
                static_cast<long>((msec % 1000) * 1000) // Microseconds
            });

            if(frame.Flags & CAN_FLAGS_EXT_ID)
            {
                msg.setExtended(true);
            }
            if(frame.Flags & CAN_FLAGS_FD)
            {
                msg.setFD(true);
            }
            if(frame.Flags & CAN_FLAGS_RTR)
            {
                msg.setRTR(true);
            }
            if(frame.Flags & CAN_FLAGS_BRS)
            {
                msg.setBRS(true);
            }
            if(frame.Flags & CAN_FLAGS_TX)
            {
                msg.setRX(false);
            }

            if(frame.ErrFlags)
            {
                qDebug() << "ERR: " << frame.ErrFlags;
                msg.setErrorFrame(true);
            }

            for(int i = 0; i < frame.DLC; i++)
            {
                msg.setByte(i, frame.Data[i]);
            }

            if(m_Channel_StatusCAN[frame.Channel])
            {
                m_ReceiveQueue[frame.Channel].push(msg);
            }

            break;
        }

        case 253u: // DATA_REPORT_LIN_MSG
        {
            /*uint8_t ch = packet.Data[1] + 1;

            uint32_t time = packet.Data[2]<<24 | packet.Data[3]<<16 | packet.Data[4]<<8 | packet.Data[5];

            uint8_t alive = packet.Data[6];
            uint8_t valid = packet.Data[7];

            uint8_t id = packet.Data[8];
            uint8_t len = packet.Data[9];
            //uint8_t crc = dat.Data[10];

            uint8_t data[8] = {};
            memcpy(data, &packet.Data[11], len);

            QVector<std::string> v;
            for(uint8_t i: data)
            {
                v.append(uint8_to_hex(i));
            }*/

            fprintf(stderr, "LIN MSG\n");
            break;
        }

        default:
            break;
        }
        break;

    case MSG_NOTIFICATION:
    {
        uint8_t type = packet.Data[0];
        Q_UNUSED(type);
        fprintf(stderr, "DEV: %s\n", (char*)&packet.Data[1]);

        break;
    }

    case MSG_RESPONSE:
        break;

    case MSG_ERROR:
        break;

    default:
        break;
    }
}


void GrIPHandler::WorkerThread()
{
    while(!m_Exit)
    {
        // Update GrIP
        for(int i = 0; i < 32; i++)
        {
            GrIP_Update();
        }

        // Check for new packet
        for(int i = 0; i < 32; i++)
        {
            GrIP_Packet_t dat;
            if(GrIP_Receive(&dat))
            {
                ProcessData(dat);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
