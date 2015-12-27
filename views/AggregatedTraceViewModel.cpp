#include "AggregatedTraceViewModel.h"

AggregatedTraceViewModel::AggregatedTraceViewModel(CanDb *candb, CanTrace *trace)
  : _candb(candb), _trace(trace)
{
    _rootItem = new AggregatedTraceViewItem(0);

    connect(trace, SIGNAL(messageEnqueued(CanMessage)), this, SLOT(messageReceived(CanMessage)));
}

void AggregatedTraceViewModel::messageReceived(const CanMessage &msg)
{
    AggregatedTraceViewItem *item, *subitem;
    struct timeval tv_last, tv_now;

    uint32_t raw_id = msg.getRawId();

    if (_map.find(raw_id) == _map.end()) {
        // create new row
        item = new AggregatedTraceViewItem(_rootItem);
        item->_name = "unknown";
        item->_lastmsg = msg;
        item->_interval.tv_sec = 0;
        item->_interval.tv_usec = 0;

        CanDbMessage *dbmsg = _candb->getMessageById(raw_id);
        if (dbmsg) {
            item->_name = dbmsg->getName();
            CanDbSignal *dbsignal;
            foreach (dbsignal, dbmsg->getSignals()) {
                subitem = new AggregatedTraceViewItem(item);
                subitem->_name = dbsignal->name();
                item->appendChild(subitem);
            }
        }

        beginResetModel();
        _rootItem->appendChild(item);
        _map[raw_id] = item;
        endResetModel();

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

        int row = item->row();
        dataChanged(createIndex(row, 0, item), createIndex(row, column_count-1, item));

    }

}


QModelIndex AggregatedTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    const AggregatedTraceViewItem *parentItem = parent.isValid() ? static_cast<AggregatedTraceViewItem*>(parent.internalPointer()) : _rootItem;
    const AggregatedTraceViewItem *childItem = parentItem->child(row);

    if (childItem) {
        return createIndex(row, column, const_cast<AggregatedTraceViewItem*>(childItem));
    } else {
        return QModelIndex();
    }
}

QModelIndex AggregatedTraceViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    AggregatedTraceViewItem *childItem = (AggregatedTraceViewItem*) index.internalPointer();
    AggregatedTraceViewItem *parentItem = childItem->parent();

    if (parentItem == _rootItem) {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}


int AggregatedTraceViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0) {
        return 0;
    }

    AggregatedTraceViewItem *parentItem;
    if (parent.isValid()) {
        parentItem = (AggregatedTraceViewItem*)parent.internalPointer();
    } else {
        parentItem = _rootItem;
    }
    return parentItem->childCount();
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
                case column_comment:
                    return QString("Comment");
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
    (void) role;

    AggregatedTraceViewItem *item = (AggregatedTraceViewItem *)index.internalPointer();

    if (item->parent() != _rootItem) {

        const CanMessage *msg = &item->parent()->_lastmsg;

        CanDbMessage *dbmsg = _candb->getMessageById(msg->getRawId());
        if (!dbmsg) { return QVariant(); }

        CanDbSignal *dbsignal = dbmsg->getSignal(item->row());
        if (!dbsignal) { return QVariant(); }

        uint32_t raw_data;
        QString value_name;
        switch (index.column()) {

            case column_name:
                return dbsignal->name();

            case column_data:
                raw_data = msg->extractSignal(dbsignal->startBit(), dbsignal->length(), false);
                value_name = dbsignal->getValueName(raw_data);

                if (value_name.isEmpty()) {
                    return raw_data;
                } else {
                    return QString().number(raw_data) + " - " + value_name;
                }

            case column_comment:
                return dbsignal->comment();

            default:
                return QVariant();

        }
    }

    const CanMessage *msg = &item->_lastmsg;
    CanDbMessage *dbmsg = _candb->getMessageById(msg->getRawId());

    struct timeval tv = item->_interval;
    double intervalD;
    switch (index.column()) {
        case column_timestamp:
            intervalD = (double)tv.tv_sec + ((double)tv.tv_usec/1000000);
            return (intervalD==0) ? "" : QString().sprintf("%.04f", intervalD);
        case column_channel:
            return msg->getInterfaceId();
        case column_direction:
            return (msg->getId() % 7)==0 ? "tx" : "rx";
        case column_canid:
            return msg->getIdString();
        case column_name:
            return (dbmsg) ? dbmsg->getName() : "";
        case column_dlc:
            return msg->getLength();
        case column_data:
            return msg->getDataHexString();
        case column_comment:
            return (dbmsg) ? dbmsg->getComment() : "";

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
        case column_comment: return Qt::AlignLeft + Qt::AlignVCenter;
       default: return QVariant();
    }
}


AggregatedTraceViewItem::AggregatedTraceViewItem(AggregatedTraceViewItem *parent)
  : _parent(parent)
{

}

AggregatedTraceViewItem::~AggregatedTraceViewItem()
{
    qDeleteAll(_children);
}

void AggregatedTraceViewItem::appendChild(AggregatedTraceViewItem *child)
{
    _children.append(child);
}

AggregatedTraceViewItem *AggregatedTraceViewItem::child(int row) const
{
    return _children.value(row);
}

int AggregatedTraceViewItem::childCount() const
{
    return _children.count();
}

int AggregatedTraceViewItem::row() const
{
    if (_parent) {
        return _parent->_children.indexOf(const_cast<AggregatedTraceViewItem*>(this));
    } else {
        return 0;
    }
}

AggregatedTraceViewItem *AggregatedTraceViewItem::parent()
{
    return _parent;
}
