#include "UnifiedTraceViewModel.h"
#include <core/CanTrace.h>
#include <core/Backend.h>
#include <QColor>
#include <QSet>
#include <QDateTime>
#include <core/ThemeManager.h>

UnifiedTraceViewModel::UnifiedTraceViewModel(Backend &backend, Category category)
    : BaseTraceViewModel(backend), m_category(category)
{
    m_rootItem = std::make_shared<UnifiedTraceItem>(CanMessage()); // Dummy root
    m_firstTimestamp = 0;
    m_previousRowTimestamp = 0;
    m_globalIndexCounter = 1;

    connect(backend.getTrace(), SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect(backend.getTrace(), SIGNAL(afterAppend()), this, SLOT(afterAppend()));
    connect(backend.getTrace(), SIGNAL(beforeRemove(int)), this, SLOT(beforeRemove(int)));
    connect(backend.getTrace(), SIGNAL(afterRemove(int)), this, SLOT(afterRemove(int)));
    connect(backend.getTrace(), SIGNAL(beforeClear()), this, SLOT(beforeClear()));
    connect(backend.getTrace(), SIGNAL(afterClear()), this, SLOT(afterClear()));
    connect(&backend, SIGNAL(onSetupChanged()), this, SLOT(onSetupChanged()));
    
    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(onResumeMeasurement()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(onPauseMeasurement()));
}

UnifiedTraceViewModel::~UnifiedTraceViewModel()
{
}

QModelIndex UnifiedTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    UnifiedTraceItem *parentItem;
    if (!parent.isValid())
        parentItem = m_rootItem.get();
    else
        parentItem = static_cast<UnifiedTraceItem*>(parent.internalPointer());

    std::shared_ptr<UnifiedTraceItem> childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem.get());
    else
        return QModelIndex();
}

QModelIndex UnifiedTraceViewModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())
        return QModelIndex();

    UnifiedTraceItem *childItem = static_cast<UnifiedTraceItem*>(child.internalPointer());
    UnifiedTraceItem *parentItem = childItem->parentItem();

    if (parentItem == m_rootItem.get())
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int UnifiedTraceViewModel::rowCount(const QModelIndex &parent) const
{
    UnifiedTraceItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_rootItem.get();
    else
        parentItem = static_cast<UnifiedTraceItem*>(parent.internalPointer());

    return parentItem->childCount();
}

int UnifiedTraceViewModel::columnCount(const QModelIndex &parent) const
{
    return BaseTraceViewModel::column_count;
}

bool UnifiedTraceViewModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

CanMessage UnifiedTraceViewModel::getMessage(const QModelIndex &index) const
{
    if (!index.isValid()) return CanMessage();
    UnifiedTraceItem *item = static_cast<UnifiedTraceItem*>(index.internalPointer());
    if (item->isProtocol()) {
        const ProtocolMessage& pmsg = item->protocolMessage();
        return pmsg.rawFrames.isEmpty() ? CanMessage() : pmsg.rawFrames.first();
    } else if (item->isMetadata()) {
        UnifiedTraceItem *parent = item->parentItem();
        if (parent && parent->isProtocol()) {
            const ProtocolMessage& pmsg = parent->protocolMessage();
            return pmsg.rawFrames.isEmpty() ? CanMessage() : pmsg.rawFrames.first();
        }
        return CanMessage();
    } else {
        return item->rawFrame();
    }
}

QVariant UnifiedTraceViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return data_DisplayRole(index);
        case Qt::ForegroundRole:
            return data_TextColorRole(index);
        case Qt::TextAlignmentRole:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        default:
            return BaseTraceViewModel::data(index, role);
    }
}

void UnifiedTraceViewModel::beforeAppend(int num_messages)
{
    // Integration with ProtocolManager happens here
    // But beginInsertRows needs to know how many PARENT rows we are adding.
    // This is tricky because one ProtocolMessage might consume multiple frames.
}

void UnifiedTraceViewModel::afterAppend()
{
    if (!m_isActive) return;
    
    CanTrace *trace = backend()->getTrace();
    int size = trace->size();
    QSet<int> updatedRows;
    if (m_lastProcessedIndex >= size) {
        m_lastProcessedIndex = size - 1;
    }

    QList<std::shared_ptr<UnifiedTraceItem>> newItems;

    for (int i = m_lastProcessedIndex + 1; i < size; ++i) {
        CanMessage msg = trace->getMessage(i);

        ProtocolMessage pmsg;
        DecodeStatus status = m_protocolManager.processFrame(msg, pmsg);
        
        bool shouldAppend = false;
        if (status == DecodeStatus::Completed) {
            if (m_category == Cat_All) {
                shouldAppend = true;
            } else if (pmsg.protocol.compare("uds", Qt::CaseInsensitive) == 0 && m_category == Cat_UDS) {
                shouldAppend = true;
            } else if (pmsg.protocol.compare("j1939", Qt::CaseInsensitive) == 0 && m_category == Cat_J1939) {
                uint32_t key = getJ1939Key(pmsg);
                if (m_j1939AggregatedMap.count(key)) {
                    auto &item = m_j1939AggregatedMap[key];
                    item->updateProtocolMessage(pmsg);
                    updatedRows.insert(item->row());
                } else {
                    shouldAppend = true;
                }
            }

            if (shouldAppend) {
                auto item = std::make_shared<UnifiedTraceItem>(pmsg, m_rootItem.get());
                if (m_firstTimestamp == 0) m_firstTimestamp = pmsg.timestamp;
                item->setTimestamp(pmsg.timestamp);
                item->setGlobalIndex(m_globalIndexCounter++); 
                newItems.append(item);
                
                if (m_category == Cat_J1939) {
                    m_j1939AggregatedMap[getJ1939Key(pmsg)] = item;
                }
            }
        } else if (status == DecodeStatus::Ignored) {
            if (m_category == Cat_All) {
                auto item = std::make_shared<UnifiedTraceItem>(msg, m_rootItem.get());
                uint64_t ts = static_cast<uint64_t>(msg.getFloatTimestamp() * 1000000.0);
                if (m_firstTimestamp == 0) m_firstTimestamp = ts;
                item->setTimestamp(ts);
                item->setGlobalIndex(m_globalIndexCounter++);
                newItems.append(item);
            }
        }
        m_lastProcessedIndex = i;
    }

    if (!newItems.isEmpty()) {
        beginInsertRows(QModelIndex(), m_rootItem->childCount(), m_rootItem->childCount() + newItems.size() - 1);
        for (auto &item : newItems) {
            m_rootItem->appendChild(item);
        }
        endInsertRows();
    }

    // Emit batched dataChanged signals for aggregated rows
    for (int row : updatedRows) {
        auto item = m_rootItem->child(row);
        if (item) {
            QModelIndex idx = createIndex(row, 0, item.get());
            emit dataChanged(idx, idx.sibling(row, column_count - 1));
            
            // Also notify children updates if expanded
            if (item->childCount() > 0) {
                QModelIndex firstChild = index(0, 0, idx);
                QModelIndex lastChild = index(item->childCount() - 1, column_count - 1, idx);
                emit dataChanged(firstChild, lastChild);
            }
        }
    }

    // Hard row limit check for the UI model
    if (m_rootItem->childCount() > m_maxRows) {
        int toRemove = m_maxRows / 10; // Remove 10%
        if (toRemove > 0) {
            beginRemoveRows(QModelIndex(), 0, toRemove - 1);
            m_rootItem->removeChildren(0, toRemove);
            // After removal, row indices of remaining items must be updated
            for (int i = 0; i < m_rootItem->childCount(); ++i) {
                m_rootItem->child(i)->setRow(i);
            }
            // Rebuild aggregated map to point to updated rows correctly after pruning
            m_j1939AggregatedMap.clear();
            for (int i = 0; i < m_rootItem->childCount(); ++i) {
                auto child = m_rootItem->child(i);
                if (child && child->isProtocol() && child->protocolMessage().protocol.compare("j1939", Qt::CaseInsensitive) == 0) {
                    m_j1939AggregatedMap[getJ1939Key(child->protocolMessage())] = child;
                }
            }
            endRemoveRows();
        }
    }
}

void UnifiedTraceViewModel::beforeRemove(int count)
{
    // Adjust lastProcessedIndex since CanTrace removed items from the front
    m_lastProcessedIndex -= count;
    if (m_lastProcessedIndex < -1) m_lastProcessedIndex = -1;
}

void UnifiedTraceViewModel::afterRemove(int count)
{
    // Nothing more needed here specifically for m_lastProcessedIndex
}

void UnifiedTraceViewModel::beforeClear()
{
    beginResetModel();
}

void UnifiedTraceViewModel::afterClear()
{
    m_rootItem = std::make_shared<UnifiedTraceItem>(CanMessage());
    m_protocolManager.reset();
    m_lastProcessedIndex = -1;
    m_globalIndexCounter = 1;
    m_firstTimestamp = 0;
    m_previousRowTimestamp = 0;
    m_j1939AggregatedMap.clear();
    endResetModel();
}

void UnifiedTraceViewModel::onSetupChanged()
{
    beginResetModel();
    m_rootItem = std::make_shared<UnifiedTraceItem>(CanMessage());
    m_protocolManager.reset();
    m_lastProcessedIndex = -1;
    m_globalIndexCounter = 1;
    m_firstTimestamp = 0;
    m_previousRowTimestamp = 0;
    m_j1939AggregatedMap.clear();
    
    // Re-process all frames from trace buffer
    // We bypass beginInsertRows by being inside beginResetModel
    afterAppend(); 
    
    endResetModel();
}

void UnifiedTraceViewModel::onResumeMeasurement()
{
    m_isActive = true;
    afterAppend();
}

void UnifiedTraceViewModel::onPauseMeasurement()
{
    m_isActive = false;
}

uint32_t UnifiedTraceViewModel::getJ1939Key(const ProtocolMessage& pmsg) const
{
    uint32_t pgn = pmsg.id;
    uint32_t sa = pmsg.metadata.value("Source Address").toUInt();
    return (pgn << 8) | (sa & 0xFF);
}

QVariant UnifiedTraceViewModel::data_DisplayRole(const QModelIndex &index) const
{
    UnifiedTraceItem *item = static_cast<UnifiedTraceItem*>(index.internalPointer());
    
    uint64_t current = item->timestamp();
    if (index.column() == column_index) {
        return (item->parentItem() == m_rootItem.get()) ? QVariant(item->globalIndex()) : QVariant();
    }

    if (item->isProtocol()) {
        const ProtocolMessage& pmsg = item->protocolMessage();
        switch (index.column()) {
            case column_timestamp: 
            {
                uint64_t prev = 0;
                if (index.row() > 0) {
                    auto prevItem = item->parentItem()->child(index.row() - 1);
                    if (prevItem) prev = prevItem->timestamp();
                } else {
                    prev = (item->parentItem() != m_rootItem.get()) ? item->parentItem()->timestamp() : 0;
                }
                return formatUnifiedTimestamp(current, prev);
            }
            case column_canid: 
            {
                uint32_t rawId = pmsg.rawFrames.isEmpty() ? 0 : pmsg.rawFrames.first().getId();
                QString rawStr = QString("0x%1").arg(rawId, 0, 16);
                if (pmsg.protocol.compare("uds", Qt::CaseInsensitive) == 0) return QString("%1 (SID:%2)").arg(rawStr).arg(pmsg.id, 2, 16, QChar('0'));
                if (pmsg.protocol.compare("j1939", Qt::CaseInsensitive) == 0) return QString("%1 (PGN:%2)").arg(rawStr).arg(pmsg.id, 0, 16);
                return rawStr;
            }
            case column_type: 
                if (pmsg.protocol.compare("j1939", Qt::CaseInsensitive) == 0) {
                    uint8_t pf = (pmsg.id >> 8) & 0xFF; // pmsg.id is PGN
                    return (pf < 240) ? "PDU1" : "PDU2";
                }
                return pmsg.protocol.toUpper();
            case column_name: return pmsg.name;
            case column_comment: return pmsg.description;
            case column_data: return pmsg.payload.toHex(' ');
            case column_dlc: return pmsg.payload.size();
            case column_direction: return pmsg.rawFrames.isEmpty() ? "" : (pmsg.rawFrames.first().isRX() ? "RX" : "TX");
            case column_channel: return pmsg.rawFrames.isEmpty() ? "" : backend()->getInterfaceName(pmsg.rawFrames.first().getInterfaceId());
            case column_sender:
                if (pmsg.protocol.compare("uds", Qt::CaseInsensitive) == 0) {
                    return (pmsg.type == MessageType::Request) ? "Tester" : "ECU";
                }
                return "";
            default: return QVariant();
        }
    } else if (item->isMetadata()) {
        switch (index.column()) {
            case column_timestamp: 
            {
                uint64_t prev = 0;
                if (index.row() > 0) {
                    auto prevItem = item->parentItem()->child(index.row() - 1);
                    if (prevItem) prev = prevItem->timestamp();
                } else {
                    prev = (item->parentItem() != m_rootItem.get()) ? item->parentItem()->timestamp() : 0;
                }
                return formatUnifiedTimestamp(current, prev);
            }
            case column_name: return item->metadataName();
            case column_data: return item->metadataValue();
            case column_type: 
                if (item->metadataName() == "Priority") return "P";
                if (item->metadataName() == "Reserved") return "R";
                if (item->metadataName() == "Data Page") return "DP";
                if (item->metadataName() == "PDU Format") return "PF";
                if (item->metadataName() == "PDU Specific") return "PS";
                if (item->metadataName() == "Source Address") return "SA";
                return "";
            default: return QVariant();
        }
    } else {
        const CanMessage& msg = item->rawFrame();
        CanDbMessage *dbmsg = backend()->findDbMessage(msg);
        switch (index.column()) {
            case column_index: 
                return (item->parentItem() == m_rootItem.get()) ? QVariant(item->globalIndex()) : QVariant();
            case column_timestamp: 
            {
                uint64_t prev = 0;
                if (index.row() > 0) {
                    auto prevItem = item->parentItem()->child(index.row() - 1);
                    if (prevItem) prev = prevItem->timestamp();
                } else {
                    prev = (item->parentItem() != m_rootItem.get()) ? item->parentItem()->timestamp() : 0;
                }
                return formatUnifiedTimestamp(current, prev);
            }
            case column_channel: return backend()->getInterfaceName(msg.getInterfaceId());
            case column_direction: return msg.isRX() ? "RX" : "TX";
            case column_type: return msg.isFD() ? "fd" : "can";
            case column_canid: return QString("0x%1").arg(msg.getId(), 0, 16);
            case column_dlc: return msg.getLength();
            case column_data: return msg.getDataHexString().toLower();
            case column_name: 
                if (item->parentItem() != m_rootItem.get()) {
                    // This is a child row - show transport layer info
                    uint8_t firstByte = msg.getByte(0);
                    uint8_t type = (firstByte >> 4) & 0x0F;
                    if (type == 0x0) return "[tp] Single Frame";
                    if (type == 0x1) return "[tp] First Frame";
                    if (type == 0x2) return QString("[tp] Consecutive Frame (SN: %1)").arg(firstByte & 0x0F);
                    if (type == 0x3) return "[tp] Flow Control";
                }
                return (dbmsg) ? dbmsg->getName() : "[raw]";
            case column_comment: return (dbmsg) ? dbmsg->getComment() : "";
            case column_sender: return ""; // Child rows don't show sender as per requirements
            default: return QVariant();
        }
    }
}

QVariant UnifiedTraceViewModel::data_TextColorRole(const QModelIndex &index) const
{
    UnifiedTraceItem *item = static_cast<UnifiedTraceItem*>(index.internalPointer());
    bool isDark = ThemeManager::instance().isDarkMode();

    if (item->isProtocol()) {
        const ProtocolMessage& pmsg = item->protocolMessage();
        switch (pmsg.type) {
            case MessageType::Request: 
                return isDark ? QColor(100, 180, 255) : QColor(0, 0, 139); 
            case MessageType::PositiveResponse: 
                return isDark ? QColor(120, 255, 120) : QColor(0, 100, 0); 
            case MessageType::NegativeResponse: 
                return isDark ? QColor(255, 120, 120) : QColor(139, 0, 0); 
            default: break;
        }
    }
    const CanMessage& msg = item->rawFrame();
    if (msg.isErrorFrame()) return isDark ? QColor(255, 100, 100) : QColor(Qt::red);
    return QVariant();
}

QString UnifiedTraceViewModel::formatUnifiedTimestamp(uint64_t ts, uint64_t prevTs) const
{
    double val = 0;
    switch (timestampMode()) {
        case timestamp_mode_absolute:
            return QDateTime::fromMSecsSinceEpoch(ts / 1000).toString("HH:mm:ss.zzz");
        case timestamp_mode_relative:
            val = (double)(ts - (uint64_t)(backend()->getTimestampAtMeasurementStart() * 1000000.0)) / 1000000.0;
            break;
        case timestamp_mode_delta:
            val = (prevTs > 0) ? (double)(ts - prevTs) / 1000000.0 : 0.0;
            break;
        default:
            return "0.000000";
    }
    return QString::number(val, 'f', 6);
}
