#include "AggregatedTraceViewModel.h"
#include <QColor>

#include <model/CanTrace.h>
#include <model/CanDbMessage.h>
#include <model/MeasurementSetup.h>

AggregatedTraceViewModel::AggregatedTraceViewModel(CanTrace *trace)
  : BaseTraceViewModel(trace)
{
    _rootItem = new AggregatedTraceViewItem(0);
    _updateTimer = new QTimer(this);
    connect(_updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTimer()));
    connect(trace, SIGNAL(messageEnqueued(CanMessage)), this, SLOT(messageReceived(CanMessage)));

    _updateTimer->setSingleShot(true);
}

void AggregatedTraceViewModel::createItem(const CanMessage &msg)
{
    AggregatedTraceViewItem *item = new AggregatedTraceViewItem(_rootItem);
    item->_lastmsg = msg;

    CanDbMessage *dbmsg = _trace->setup()->findDbMessage(&msg);
    if (dbmsg) {
        for (int i=0; i<dbmsg->getSignals().length(); i++) {
            item->appendChild(new AggregatedTraceViewItem(item));
        }
    }

    _rootItem->appendChild(item);
    _map[makeUniqueKey(msg)] = item;
}

void AggregatedTraceViewModel::updateItem(const CanMessage &msg)
{
    struct timeval tv_last, tv_now;

    AggregatedTraceViewItem *item = _map.value(makeUniqueKey(msg));
    if (item) {
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
}

void AggregatedTraceViewModel::onUpdateTimer()
{

    if (_pendingMessageUpdates.length()) {
        foreach (CanMessage msg, _pendingMessageUpdates) {
            updateItem(msg);
        }
        _pendingMessageUpdates.clear();
    }

    if (_pendingMessageInserts.length()) {
        beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount()+_pendingMessageInserts.length()-1);
        foreach (CanMessage msg, _pendingMessageInserts) {
            createItem(msg);
        }
        endInsertRows();
        _pendingMessageInserts.clear();
    }

    if (_rootItem->childCount()>0) {
        dataChanged(createIndex(0, 0, _rootItem->firstChild()), createIndex(_rootItem->childCount()-1, column_count-1, _rootItem->lastChild()));
    }

}

void AggregatedTraceViewModel::messageReceived(const CanMessage &msg)
{
    if (_map.contains(makeUniqueKey(msg))) {
        _pendingMessageUpdates.append(msg);
    } else {
        _pendingMessageInserts.append(msg);
    }

    if (!_updateTimer->isActive()) {
        _updateTimer->start(100);
    }
}

AggregatedTraceViewModel::unique_key_t AggregatedTraceViewModel::makeUniqueKey(const CanMessage &msg)
{
    return ((uint64_t)msg.getInterface()->getId() << 32) | msg.getRawId();
}

double AggregatedTraceViewModel::getTimeDiff(const timeval t1, const timeval t2) const
{
    double diff = t2.tv_sec - t1.tv_sec;
    diff += ((double)(t2.tv_usec - t1.tv_usec)) / 1000000;
    return diff;
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

QVariant AggregatedTraceViewModel::data_TextColorRole(const QModelIndex &index, int role) const
{
    (void) role;
    AggregatedTraceViewItem *item = (AggregatedTraceViewItem *)index.internalPointer();
    if (!item) { return QVariant(); }

    struct timeval now;
    gettimeofday(&now, 0);

    int color = getTimeDiff(item->_lastmsg.getTimestamp(), now)*100;
    if (color>200) { color = 200; }

    return QVariant::fromValue(QColor(color, color, color));
}


