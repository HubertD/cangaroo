#pragma once

#include <QString>
#include <QByteArray>
#include <QVector>
#include <core/CanMessage.h>

#include <QVariantMap>

enum class MessageType {
    Request,
    PositiveResponse,
    NegativeResponse,
    Unknown
};

struct ProtocolMessage {
    QString name;          // e.g., "Read VIN"
    QString description;   // Details or NRC info
    uint64_t timestamp;
    QByteArray payload;    // Reassembled data
    QVector<CanMessage> rawFrames; // The frames that made this up
    QString protocol;      // "UDS", "J1939"
    MessageType type = MessageType::Unknown;
    uint32_t id;           // SID or PGN
    uint32_t globalIndex = 0;
    QVariantMap metadata;  // Protocol specific fields (Priority, SA, DA, etc.)
};
