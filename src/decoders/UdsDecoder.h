#pragma once

#include "IDecoder.h"
#include <QMap>

class UdsDecoder : public IDecoder {
public:
    UdsDecoder();
    virtual ~UdsDecoder() = default;

    virtual DecodeStatus tryDecode(const CanMessage& frame, ProtocolMessage& outMsg) override;
    virtual void reset() override;

private:
    struct IsotpSession {
        QVector<CanMessage> frames;
        QByteArray data;
        int expectedSize = 0;
        int nextSn = 1;
        uint32_t rxId = 0;
    };

    QMap<uint32_t, IsotpSession> m_sessions;

    QString interpretService(uint8_t sid);
    QString interpretNrc(uint8_t nrc);
};
