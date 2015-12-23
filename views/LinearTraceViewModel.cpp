#include "LinearTraceViewModel.h"
#include <iostream>
#include <stddef.h>

LinearTraceViewModel::LinearTraceViewModel(CanTrace *trace)
  : _trace(trace)
{
    connect(_trace, SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect(_trace, SIGNAL(afterAppend(int)), this, SLOT(afterAppend(int)));
}

QModelIndex LinearTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalId()) {
        return createIndex(row, column, 0x80000000 | parent.internalId());
    } else {
        return createIndex(row, column, row+1);
    }
}

QModelIndex LinearTraceViewModel::parent(const QModelIndex &child) const
{
    (void) child;
    quintptr id = child.internalId();
    if (id & 0x80000000) {
        return createIndex(id & 0x7FFFFFFF, 0, id & 0x7FFFFFFF);
    }
    return QModelIndex();
}

int LinearTraceViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        quintptr id = parent.internalId();
        if (id & 0x80000000) { // node of a message
            return 0;
        } else { // a message
            const CanMessage *msg = _trace->getMessage(id-1);
            if (msg) {
                return msg->getLength();
            } else {
                return 0;
            }
        }
    } else {
        return _trace->size();
    }
}

int LinearTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 7;
}

bool LinearTraceViewModel::hasChildren(const QModelIndex &parent) const
{
    quintptr id = parent.internalId();
    if (id) {
        if (id & 0x80000000) {
            return false;
        } else {
            return _trace->getMessage(id-1)->getLength()>0;
        }
    } else {
        return !parent.isValid();
    }
}


QVariant LinearTraceViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case column_timestamp:
                    return QString("Timestamp");
                case column_channel:
                    return QString("Channel");
                case column_direction:
                    return QString("Rx/Tx");
                case column_canid:
                    return QString("CAN ID");
                case column_name:
                    return QString("Name");
                case column_dlc:
                    return QString("DLC");
                case column_data:
                    return QString("Data");
            }
        }

    }
    return QVariant();
}

void LinearTraceViewModel::beforeAppend(int num_messages)
{
    beginInsertRows(QModelIndex(), _trace->size(), _trace->size()+num_messages-1);
}

void LinearTraceViewModel::afterAppend(int num_messages)
{
    (void) num_messages;
    endInsertRows();
}

QVariant LinearTraceViewModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return data_DisplayRole(index, role);
        case Qt::TextAlignmentRole:
            return data_TextAlignmentRole(index, role);
        default:
            return QVariant();
    }
}

QVariant LinearTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) role;

    quintptr id = index.internalId();

    if (id & 0x80000000) {
        switch (index.column()) {
            case column_canid: return QString().sprintf("row %d", index.row());
            case column_dlc: return QString().sprintf("id 0x%08X", (uint32_t)index.parent().internalId());
            case column_data: return QString().sprintf("id 0x%08X", (uint32_t)index.internalId());
        }

    } else if (id) {
        const CanMessage *msg = _trace->getMessage(id-1);
        if (msg) {
            struct timeval tv = msg->getTimestamp();
            switch (index.column()) {
                case column_timestamp: return QString().sprintf("%.04f", ((double)tv.tv_sec + ((double)tv.tv_usec/1000000)));
                case column_channel: return msg->getInterfaceId();
                case column_direction: return (msg->getId() % 7)==0 ? "tx" : "rx";
                case column_canid: return QString().sprintf("0x%08X", msg->getId());
                case column_dlc: return msg->getLength();
                case column_data: return msg->getDataHexString();
            }
        }
    }

    return QVariant();
}

QVariant LinearTraceViewModel::data_TextAlignmentRole(const QModelIndex &index, int role) const
{
    (void) role;
    switch (index.column()) {
        case column_timestamp: return Qt::AlignRight + Qt::AlignVCenter;
        case column_channel: return Qt::AlignCenter + Qt::AlignVCenter;
        case column_direction: return Qt::AlignCenter + Qt::AlignVCenter;
        case column_canid: return Qt::AlignRight + Qt::AlignVCenter;
        case column_name: return Qt::AlignLeft + Qt::AlignVCenter;
        case column_dlc: return Qt::AlignCenter + Qt::AlignVCenter;
        case column_data: return Qt::AlignLeft + Qt::AlignVCenter;
       default: return QVariant();
    }
}
