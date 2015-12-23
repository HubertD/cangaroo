#include "AggregatedTraceViewModel.h"

AggregatedTraceViewModel::AggregatedTraceViewModel(CanTrace *trace)
  : _trace(trace)
{
    connect(trace, SIGNAL(messageEnqueued(CanMessage)), this, SLOT(messageReceived(CanMessage)));
}

void AggregatedTraceViewModel::messageReceived(const CanMessage &msg)
{
    AggregatedTraceViewItem *item;
    struct timeval tv_last, tv_now;

    uint32_t raw_id = msg.getRawId();
    beginResetModel();

    if (_map.find(raw_id) == _map.end()) {
        // create new row
        item = new AggregatedTraceViewItem();
        item->_lastmsg = msg;
        item->_interval.tv_sec = 0;
        item->_interval.tv_usec = 0;

        _list.push_back(item);
        _map[raw_id] = item;

    } else {

        // update row
        item = _map[raw_id];
        tv_last = item->_lastmsg.getTimestamp();
        tv_now = msg.getTimestamp();
        item->_lastmsg = msg;

        int diff_us = tv_now.tv_usec - tv_last.tv_usec;
        item->_interval.tv_sec = tv_now.tv_sec - tv_last.tv_sec;
        if (diff_us<0) {
            item->_interval.tv_sec--;
            diff_us += 1000000;
        }
        item->_interval.tv_usec = diff_us;

    }

    endResetModel();
}


QModelIndex AggregatedTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    (void) parent;
    if (row<_list.size()) {
        return createIndex(row, column, _list[row]);
    } else {
        return createIndex(row, column);
    }
}

QModelIndex AggregatedTraceViewModel::parent(const QModelIndex &child) const
{
    (void) child;
    return QModelIndex();
}

bool AggregatedTraceViewModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}

int AggregatedTraceViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return _list.size();
    }
}

int AggregatedTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

QVariant AggregatedTraceViewModel::headerData(int section, Qt::Orientation orientation, int role) const
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

QVariant AggregatedTraceViewModel::data(const QModelIndex &index, int role) const
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

QVariant AggregatedTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;

    if (index.row()>=_list.size()) {
        return QVariant();
    }

    AggregatedTraceViewItem *item = _list[index.row()];

    const CanMessage *msg = &item->_lastmsg;

    struct timeval tv = item->_interval;
    switch (index.column()) {
        case column_timestamp: return QString().sprintf("%.04f", ((double)tv.tv_sec + ((double)tv.tv_usec/1000000)));
        case column_channel: return msg->getInterfaceId();
        case column_direction: return (msg->getId() % 7)==0 ? "tx" : "rx";
        case column_canid: return QString().sprintf("0x%08X", msg->getId());
        case column_dlc: return msg->getLength();
        case column_data: return msg->getDataHexString();
    }


    return QVariant();
}

QVariant AggregatedTraceViewModel::data_TextAlignmentRole(const QModelIndex &index, int role) const
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

