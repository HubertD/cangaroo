#include "AggregatedTraceViewModel.h"

AggregatedTraceViewModel::AggregatedTraceViewModel(MeasurementSetup *setup)
  : BaseTraceViewModel(setup)
{
    _rootItem = new AggregatedTraceViewItem(0);
    connect(_setup->getTrace(), SIGNAL(messageEnqueued(CanMessage)), this, SLOT(messageReceived(CanMessage)));
}

void AggregatedTraceViewModel::messageReceived(const CanMessage &msg)
{
    AggregatedTraceViewItem *item;
    struct timeval tv_last, tv_now;

    // FIXME rawid is only unique per channel/network, not for the whole measurement
    unique_key_t key = makeUniqueKey(msg);

    if (!_map.contains(key)) {

        // create new row
        item = new AggregatedTraceViewItem(_rootItem);
        item->_lastmsg = msg;

        CanDbMessage *dbmsg = _setup->findDbMessage(&msg);
        if (dbmsg) {
            for (int i=0; i<dbmsg->getSignals().length(); i++) {
                item->appendChild(new AggregatedTraceViewItem(item));
            }
        }

        beginResetModel();
        _rootItem->appendChild(item);
        _map[key] = item;
        endResetModel();

    } else {

        // update row

        item = _map[key];
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

AggregatedTraceViewModel::unique_key_t AggregatedTraceViewModel::makeUniqueKey(const CanMessage &msg)
{
    return ((uint64_t)msg.getInterface()->getId() << 32) | msg.getRawId();
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

QVariant AggregatedTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    AggregatedTraceViewItem *item = (AggregatedTraceViewItem *)index.internalPointer();
    if (!item) { return QVariant(); }

    if (item->parent() == _rootItem) { // CanMessage row
        return data_DisplayRole_Message(index, role, &item->_lastmsg, item->_interval);
    } else { // CanSignal Row
        return data_DisplayRole_Signal(index, role, &item->parent()->_lastmsg);
    }
}


