#include "GrIP.h"
#include "CRC.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue>
#include <QDebug>

std::queue<GrIP_Packet_t> q;

// Magic byte - Marks start of transmission
#define GRIP_SOH                0x01
#define GRIP_SOT                0x02
#define GRIP_EOT                0x03


// Size of header packet
#define GRIP_HEADER_SIZE        (sizeof(GrIP_PacketHeader_t))

#define MAX_READ_CNT            128u


// GrIP states
typedef enum
{
    GrIP_State_Idle = 0, GrIP_State_RX_Header, GrIP_State_SOT, GrIP_State_RX_Data, GrIP_State_Finish
} GrIP_States_e;


// Local Prototypes
static uint8_t CheckHeader(const GrIP_PacketHeader_t *paket);
static void ForwardPacket(const GrIP_PacketHeader_t *header, const uint8_t *data, uint16_t len);

static uint8_t hex2digit(char c);
static uint32_t hex2dec(const char *str, uint8_t len);
static char nibble2hex(const uint8_t b);


// Local Variables
static GrIP_PacketHeader_t TX_Header;

// Transmit Buffer
static uint8_t TX_Buffer[GRIP_BUFFER_SIZE*2 + GRIP_HEADER_SIZE];
// Receive Data Array
static GrIP_Packet_t RX_Buff = {};
static uint8_t RxBuffer[GRIP_BUFFER_SIZE] = {};

static GrIP_States_e GrIP_Status = GrIP_State_Idle;
static bool SendReponse = false;
static uint8_t MaxReadCount = MAX_READ_CNT;
static uint32_t BytesRead = 0u;

static GrIP_ErrorFlags_t ErrorFlags;
static int Response = -1;

static QSerialPort *serPort;


void GrIP_Init(QSerialPort &serial)
{
    // Initialize to default values
    GrIP_Status = GrIP_State_Idle;
    SendReponse = false;
    MaxReadCount = MAX_READ_CNT;
    BytesRead = 0u;
    Response = -1;

    serPort = &serial;

    memset(&TX_Header, 0u, GRIP_HEADER_SIZE);
    memset(TX_Buffer, 0u, GRIP_BUFFER_SIZE + GRIP_HEADER_SIZE);
    memset(&RX_Buff, 0u, sizeof(RX_Buff));
    memset(&ErrorFlags, 0u, sizeof(GrIP_ErrorFlags_t));
    memset(RxBuffer, 0u, sizeof(RxBuffer));
}


uint8_t GrIP_TransmitArray(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const uint8_t *data, uint16_t len)
{
    GrIP_Pdu_t pdu = {(uint8_t*)data, len};

    return GrIP_Transmit(ProtType, MsgType, ReturnCode, &pdu);
}


uint8_t GrIP_Transmit(GrIP_ProtocolType_e ProtType, GrIP_MessageType_e MsgType, GrIP_ReturnType_e ReturnCode, const GrIP_Pdu_t *data)
{
    TX_Header.Version = GRIP_VERSION;
    TX_Header.Protocol = ProtType;
    TX_Header.MsgType = MsgType;
    TX_Header.ReturnCode = ReturnCode;

    if(data)
    {
        // Convert length to network order
        TX_Header.Length = data->Length;

        // Calculate header CRC without CRC fields
        TX_Header.CRC_Header = CRC_CalculateCRC8((uint8_t*)&TX_Header, GRIP_HEADER_SIZE-2u);

        // Check if data fits into transmit buffer
        if(data->Length > GRIP_BUFFER_SIZE)
        {
            return RET_NOK;
        }
        else if(data->Length > 0u)
        {
            // Calculate CRC of data
            TX_Header.CRC_Data = CRC_CalculateCRC8(data->Data, data->Length);
        }
        else
        {
            // No data, no CRC
            TX_Header.CRC_Data = 0u;
        }

        // Prepare transmit buffer
        unsigned int idx = 0u;
        uint8_t *pHeader = (uint8_t*)&TX_Header;

        // Start of header
        TX_Buffer[idx++] = GRIP_SOH;

        for(uint8_t i = 0; i < GRIP_HEADER_SIZE; i++)
        {
            // Serialize header
            TX_Buffer[idx++] = nibble2hex(pHeader[i]>>4);
            TX_Buffer[idx++] = nibble2hex(pHeader[i]);
        }

        // Start of text
        TX_Buffer[idx++] = GRIP_SOT;

        for(uint8_t i = 0; i < data->Length; i++)
        {
            // Serialize data
            TX_Buffer[idx++] = nibble2hex(data->Data[i]>>4);
            TX_Buffer[idx++] = nibble2hex(data->Data[i]);
        }

        // End of transmission
        TX_Buffer[idx++] = GRIP_EOT;

        // Transmit packet
        int d = serPort->write((char*)TX_Buffer, idx);
        serPort->flush();

        return RET_OK;
    }
    else
    {
        // No data available -> Response, Sync
        // No data to transmit, only header
        TX_Header.Length = 0u;
        TX_Header.CRC_Data = 0u;

        // Calculate header CRC without CRC fields
        TX_Header.CRC_Header = CRC_CalculateCRC8((uint8_t*)&TX_Header, GRIP_HEADER_SIZE-2u);

        unsigned int idx = 0u;
        const uint8_t *pHeader = (uint8_t*)&TX_Header;

        // Start of header
        TX_Buffer[idx++] = GRIP_SOH;

        for(uint8_t i = 0; i < GRIP_HEADER_SIZE; i++)
        {
            // Serialize header
            TX_Buffer[idx++] = nibble2hex(pHeader[i]>>4);
            TX_Buffer[idx++] = nibble2hex(pHeader[i]);
        }

        // End of transmission
        TX_Buffer[idx++] = GRIP_EOT;

        // Transmit packet
        int d = serPort->write((char*)TX_Buffer, idx);
        serPort->flush();

        return RET_OK;
    }

    // Clear memory
    memset(&TX_Header, 0u, GRIP_HEADER_SIZE);
    memset(TX_Buffer, 0u, GRIP_BUFFER_SIZE);

    return RET_NOK;
}


uint8_t GrIP_SendSync(void)
{
    return GrIP_Transmit(PROT_GrIP, MSG_SYNC, RET_OK, 0);
}


void GrIP_Update(void)
{
    serPort->waitForReadyRead(0);

    switch(GrIP_Status)
    {
    case GrIP_State_Idle:
        // Check if data is available
        if(serPort->bytesAvailable())
        {
            uint8_t magic = 0u;

            // Read start byte
            serPort->read((char*)&magic, 1u);

            if(magic == GRIP_SOH)
            {
                // Received valid packet
                GrIP_Status = GrIP_State_RX_Header;
                ErrorFlags.LastError = 0u;
                memset(&RX_Buff, 0u, sizeof(RX_Buff));
            }
        }
        break;

    case GrIP_State_RX_Header:
        // Check if header data is available
        if(serPort->bytesAvailable() > static_cast<qint64>(GRIP_HEADER_SIZE*2u-1u))
        {
            uint8_t head_buff[GRIP_HEADER_SIZE*2u] = {};
            uint8_t *pHeader = (uint8_t*)&RX_Buff.RX_Header;

            // Get header
            serPort->read((char*)head_buff, GRIP_HEADER_SIZE*2u);

            // Fill struct
            for(unsigned int i = 0u; i < GRIP_HEADER_SIZE; i++)
            {
                pHeader[i] = hex2dec((char*)&head_buff[i*2u], 2u);
            }

            // Check if header is valid
            uint8_t ret = CheckHeader(&RX_Buff.RX_Header);
            if(ret != RET_OK)
            {
                // Header is invalid
                GrIP_Status = GrIP_State_Finish;
                // Send NOK
                ErrorFlags.LastError = ret;
                //printf("Wrong header: %d\n", ret);
                qDebug() << "Wrong header: " << ret;
                break;
            }
            if(RX_Buff.RX_Header.Length > GRIP_BUFFER_SIZE)
            {
                // Payload too big
                GrIP_Status = GrIP_State_Finish;
                // Send NOK
                ErrorFlags.LastError = RET_WRONG_LEN;
                ErrorFlags.Len_Error++;
                //printf("Payload exceeds limit: %d\n", RX_Buff[GrIP_idx].RX_Header.Length);
                qDebug() << "Payload exceeds limit: " << RX_Buff.RX_Header.Length;
                break;
            }

            // If response received
            if(RX_Buff.RX_Header.MsgType == MSG_RESPONSE || RX_Buff.RX_Header.MsgType == MSG_SYNC)
            {
                GrIP_Status = GrIP_State_Finish;
                SendReponse = false;
                // check response
                //printf("Received resp cnt: %d\n", GrIP_ResponseCnt);
                if(RX_Buff.RX_Header.ReturnCode != RET_OK)
                {
                    // Response not OK
                    qDebug() << "Response: " << RX_Buff.RX_Header.ReturnCode;
                    Response = RX_Buff.RX_Header.ReturnCode;
                }
            }
            else if(RX_Buff.RX_Header.Length > 0u)
            {
                // Payload is available
                GrIP_Status = GrIP_State_SOT;
            }
            else
            {
                // No payload
                GrIP_Status = GrIP_State_Finish;
                // Send OK
                ErrorFlags.LastError = RET_OK;

                ForwardPacket(&RX_Buff.RX_Header, RX_Buff.Data, RX_Buff.RX_Header.Length);
                q.push(RX_Buff);
            }
        }
        break;

    case GrIP_State_SOT:
        if(serPort->bytesAvailable())
        {
            uint8_t magic = 0u;

            // Read Magic byte
            serPort->read((char*)&magic, 1u);
            if(magic == GRIP_SOT)
            {
                // Start of text
                GrIP_Status = GrIP_State_RX_Data;
                MaxReadCount = MAX_READ_CNT;
                BytesRead = 0u;
                memset(RxBuffer, 0u, sizeof(RxBuffer));
            }
            if(magic == GRIP_EOT || (isxdigit(magic) != 0u))
            {
                // Something went wrong, go back to idle
                GrIP_Status = GrIP_State_Idle;
                qDebug() << "SOT failed";
            }
        }
        break;

    case GrIP_State_RX_Data:
        // Check if data is available
        while((serPort->bytesAvailable() > 1u) && MaxReadCount--)
        {
            uint8_t tmp[2u] = {};
            // Get payload
            serPort->read((char*)tmp, 2u);

            if(isxdigit(tmp[0u]) && isxdigit(tmp[1u]))
            {
                // Received valid hex character
                RX_Buff.Data[BytesRead] = hex2dec((char*)tmp, 2u);
            }
            else
            {
                // Received non-hex character; exit
                ErrorFlags.LastError = RET_WRONG_PARAM;
                GrIP_Status = GrIP_State_Finish;
                qDebug() << "Rec non-hex char";

                if(RX_Buff.RX_Header.MsgType != MSG_RESPONSE)
                {
                    SendReponse = true;
                }

                if(tmp[0u] == GRIP_EOT || tmp[1u] == GRIP_EOT)
                {
                    GrIP_Status = GrIP_State_Idle;
                }

                break;
            }

            BytesRead++;

            if(BytesRead >= RX_Buff.RX_Header.Length)
            {
                GrIP_Status = GrIP_State_Finish;

                // Check CRC
                if(RX_Buff.RX_Header.CRC_Data == CRC_CalculateCRC8(RX_Buff.Data, RX_Buff.RX_Header.Length))
                {
                    ErrorFlags.LastError = RET_OK;

                    ForwardPacket(&RX_Buff.RX_Header, RX_Buff.Data, RX_Buff.RX_Header.Length);
                    q.push(RX_Buff);

                    if((RX_Buff.RX_Header.MsgType != MSG_DATA_NO_RESPONSE) && (RX_Buff.RX_Header.MsgType != MSG_RESPONSE))
                    {
                        // Send OK
                        SendReponse = true;
                    }
                }
                else
                {
                    // Wrong CRC
                    ErrorFlags.LastError = RET_WRONG_CRC;
                    ErrorFlags.CRC_Error++;
                    qDebug() << "Wrong CRC";
                }

                // Leave loop after all bytes received
                break;
            }
        }
        // Reset counter for next round
        MaxReadCount = MAX_READ_CNT;

        break;

    case GrIP_State_Finish:
    {
        if(SendReponse)
        {
            // Send response
            GrIP_Transmit(PROT_GrIP, MSG_RESPONSE, (GrIP_ReturnType_e)ErrorFlags.LastError, 0u);
            SendReponse = false;
        }

        // Read Magic byte
        uint8_t magic = 0u;
        serPort->read((char*)&magic, 1u);
        if(magic == GRIP_EOT)
        {
            // Received EOT
        }
        else
        {
            qDebug() << "EOT failed";
        }

        GrIP_Status = GrIP_State_Idle;
        break;
    }

    default:
        // Unknown state, go back to idle
        GrIP_Status = GrIP_State_Idle;
        break;
    }
}


bool GrIP_Receive(GrIP_Packet_t *p)
{
    if(q.size() > 0)
    {
        memcpy(p, &q.front(), sizeof(GrIP_Packet_t));
        q.pop();
        return true;
    }

    return false;
}


void GrIP_GetError(GrIP_ErrorFlags_t *ef)
{
    if(ef)
    {
        memcpy(ef, &ErrorFlags, sizeof(GrIP_ErrorFlags_t));
    }
}


int GrIP_GetLastResponse(void)
{
    if(Response != -1)
    {
        int tmp = Response;
        Response = -1;
        return tmp;
    }

    return -1;
}


static uint8_t CheckHeader(const GrIP_PacketHeader_t *paket)
{
    // Check NULL
    if(paket == NULL)
    {
        return RET_WRONG_PARAM;
    }

    if(paket->Version != GRIP_VERSION)
    {
        // Wrong version
        return RET_WRONG_VERSION;
    }

    if(paket->MsgType >= MSG_MAX_NUM)
    {
        // Wrong message type
        return RET_WRONG_TYPE;
    }

    if(paket->CRC_Header != CRC_CalculateCRC8((uint8_t*)paket, GRIP_HEADER_SIZE-2u))
    {
        ErrorFlags.CRC_Error++;
        // Header CRC wrong
        return RET_WRONG_CRC;
    }

    // OK
    return RET_OK;
}


static void ForwardPacket(const GrIP_PacketHeader_t *header, const uint8_t *data, uint16_t len)
{
    switch(header->Protocol)
    {
    case PROT_GrIP:
        //Protocol_ProcessMsg(data, len);
        break;

    case PROT_BoOTA:

        break;

    default:
        // Unknown protocol
        break;
    }
}


// Convert single hex char to decimal
static uint8_t hex2digit(char c)
{
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 0xA;
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 0xa;
    }

    return (c - '0') & 0xF;
}


// Convert hex string do decimal e.g.: "FF" -> len 2 = 255
static uint32_t hex2dec(const char *str, uint8_t len)
{
    uint32_t val = 0u;

    if(str)
    {
        for(uint8_t i = 0u; i < len; i++)
        {
            val = (val << 4u) | hex2digit(str[i]);
        }
    }

    return val;
}


// Convert half-byte (4bit) to single hex char
static char nibble2hex(const uint8_t b)
{
    static const char *hex_tbl = "0123456789ABCDEF";

    return hex_tbl[b & 0x0F];
}
