#include "AggregatedTraceViewModel.h"
#include <QColor>

AggregatedTraceViewModel::AggregatedTraceViewModel(MeasurementSetup *setup)
  : BaseTraceViewModel(setup)
{
    _rootItem = new AggregatedTraceViewItem(0);
    _fadeoutTimer = new QTimer(this);
    connect(_fadeoutTimer, SIGNAL(timeout()), this, SLOT(onFadeoutTimer()));
    connect(_setup->getTrace(), SIGNAL(messageEnqueued(CanMessage)), this, SLOT(messageReceived(CanMessage)));

    _fadeoutTimer->setInterval(500);
    _fadeoutTimer->start();
}

void AggregatedTraceViewModel::createItem(const CanMessage &msg)
{
    AggregatedTraceViewItem *item = new AggregatedTraceViewItem(_rootItem);
    item->_lastmsg = msg;

    CanDbMessage *dbmsg = _setup->findDbMessage(&msg);
    if (dbmsg) {
        for (int i=0; i<dbmsg->getSignals().length(); i++) {
            item->appendChild(new AggregatedTraceViewItem(item));
        }
    }

    beginResetModel();
    _rootItem->appendChild(item);
    _map[makeUniqueKey(msg)] = item;
    endResetModel();
}

void AggregatedTraceViewModel::updateItem(const CanMessage &msg)
{
    struct timeval tv_last, tv_now;

    if (!_map.contains(makeUniqueKey(msg))) { return; }
    AggregatedTraceViewItem *item = _map[makeUniqueKey(msg)];

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

void AggregatedTraceViewModel::messageReceived(const CanMessage &msg)
{

    if (_map.contains(makeUniqueKey(msg))) {
        updateItem(msg);
    } else {
        createItem(msg);
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


void AggregatedTraceViewModel::onFadeoutTimer()
{
    if (_rootItem->childCount()>0) {
        dataChanged(createIndex(0, 0, _rootItem->firstChild()), createIndex(_rootItem->childCount()-1, column_count-1, _rootItem->lastChild()));
    }
}
