#pragma once

#include <QVector>
#include <memory>
#include <core/CanMessage.h>
#include <decoders/ProtocolTypes.h>

class UnifiedTraceItem {
public:
    UnifiedTraceItem(const CanMessage& frame, UnifiedTraceItem* parent = nullptr);
    UnifiedTraceItem(const ProtocolMessage& msg, UnifiedTraceItem* parent = nullptr);
    UnifiedTraceItem(const QString& name, const QString& value, UnifiedTraceItem* parent = nullptr);
    ~UnifiedTraceItem();

    void updateProtocolMessage(const ProtocolMessage& msg);

    void appendChild(std::shared_ptr<UnifiedTraceItem> child);
    void removeChildren(int row, int count);
    std::shared_ptr<UnifiedTraceItem> child(int row);
    int childCount() const;
    int row() const;
    void setRow(int row) { m_row = row; }
    UnifiedTraceItem* parentItem();

    bool isProtocol() const { return m_isProtocol; }
    bool isMetadata() const { return m_isMetadata; }
    const CanMessage& rawFrame() const { return m_rawFrame; }
    const ProtocolMessage& protocolMessage() const { return m_protocolMessage; }
    QString metadataName() const { return m_metadataName; }
    QString metadataValue() const { return m_metadataValue; }

    uint32_t globalIndex() const { return m_globalIndex; }
    void setGlobalIndex(uint32_t index) { m_globalIndex = index; }
    uint64_t timestamp() const { return m_timestamp; }
    void setTimestamp(uint64_t ts) { m_timestamp = ts; }

private:
    QVector<std::shared_ptr<UnifiedTraceItem>> m_childItems;
    UnifiedTraceItem* m_parentItem;
    
    bool m_isProtocol;
    bool m_isMetadata = false;
    CanMessage m_rawFrame;
    ProtocolMessage m_protocolMessage;
    QString m_metadataName;
    QString m_metadataValue;

    uint32_t m_globalIndex = 0;
    uint64_t m_timestamp = 0;
    int m_row = -1;
};
