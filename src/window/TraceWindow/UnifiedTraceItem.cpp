#include "UnifiedTraceItem.h"

UnifiedTraceItem::UnifiedTraceItem(const CanMessage& frame, UnifiedTraceItem* parent)
    : m_parentItem(parent), m_isProtocol(false), m_rawFrame(frame), m_globalIndex(0), m_row(-1)
{
    m_timestamp = static_cast<uint64_t>(frame.getFloatTimestamp() * 1000000.0);
}

UnifiedTraceItem::UnifiedTraceItem(const ProtocolMessage& msg, UnifiedTraceItem* parent)
    : m_parentItem(parent), m_isProtocol(true), m_protocolMessage(msg), m_row(-1)
{
    m_timestamp = msg.timestamp;

    // Create children for metadata if any
    if (!msg.metadata.isEmpty()) {
        for (auto it = msg.metadata.begin(); it != msg.metadata.end(); ++it) {
            appendChild(std::make_shared<UnifiedTraceItem>(it.key(), it.value().toString(), this));
        }
    }

    // Create children for each raw frame
    for (const auto& frame : msg.rawFrames) {
        appendChild(std::make_shared<UnifiedTraceItem>(frame, this));
    }
}

UnifiedTraceItem::UnifiedTraceItem(const QString& name, const QString& value, UnifiedTraceItem* parent)
    : m_parentItem(parent), m_isProtocol(false), m_isMetadata(true), m_metadataName(name), m_metadataValue(value), m_row(-1)
{
    if (m_parentItem) {
        m_timestamp = m_parentItem->timestamp();
    }
}

UnifiedTraceItem::~UnifiedTraceItem()
{
}

void UnifiedTraceItem::updateProtocolMessage(const ProtocolMessage& msg)
{
    m_protocolMessage = msg;
    m_timestamp = msg.timestamp;

    // Update metadata children if they exist
    if (!msg.metadata.isEmpty()) {
        for (auto it = msg.metadata.begin(); it != msg.metadata.end(); ++it) {
            // Find existing metadata child by name
            for (auto& child : m_childItems) {
                if (child->m_isMetadata && child->m_metadataName == it.key()) {
                    child->m_metadataValue = it.value().toString();
                    child->m_timestamp = m_timestamp;
                    break;
                }
            }
        }
    }

    // Update raw frame children
    // Note: We assume the number/order of raw frames doesn't change for the same PGN+SA aggregation
    // If it does (e.g. multi-frame vs single-frame), we might need more complex logic.
    int frameIdx = 0;
    for (auto& child : m_childItems) {
        if (!child->m_isProtocol && !child->m_isMetadata) {
            if (frameIdx < msg.rawFrames.size()) {
                child->m_rawFrame = msg.rawFrames.at(frameIdx);
                child->m_timestamp = static_cast<uint64_t>(child->m_rawFrame.getFloatTimestamp() * 1000000.0);
                frameIdx++;
            }
        }
    }
}

void UnifiedTraceItem::appendChild(std::shared_ptr<UnifiedTraceItem> child)
{
    child->setRow(m_childItems.size());
    m_childItems.append(child);
}

void UnifiedTraceItem::removeChildren(int row, int count)
{
    if (row >= 0 && (row + count) <= m_childItems.size()) {
        m_childItems.remove(row, count);
    }
}

std::shared_ptr<UnifiedTraceItem> UnifiedTraceItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int UnifiedTraceItem::childCount() const
{
    return m_childItems.size();
}

int UnifiedTraceItem::row() const
{
    return m_row;
}

UnifiedTraceItem* UnifiedTraceItem::parentItem()
{
    return m_parentItem;
}
