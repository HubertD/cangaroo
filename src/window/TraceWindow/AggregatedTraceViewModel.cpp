/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "AggregatedTraceViewModel.h"
#include <QColor>
#include <QSet>
#include <core/ThemeManager.h>

#include <core/Backend.h>
#include <core/CanTrace.h>
#include <core/CanDbMessage.h>

AggregatedTraceViewModel::AggregatedTraceViewModel(Backend &backend)
  : BaseTraceViewModel(backend)
{
    _rootItem = new AggregatedTraceViewItem(0);
    connect(backend.getTrace(), SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect(backend.getTrace(), SIGNAL(beforeClear()), this, SLOT(beforeClear()));
    connect(backend.getTrace(), SIGNAL(afterClear()), this, SLOT(afterClear()));

    connect(&backend, SIGNAL(onSetupChanged()), this, SLOT(onSetupChanged()));
}

void AggregatedTraceViewModel::createItem(const CanMessage &msg)
{
    AggregatedTraceViewItem *item = new AggregatedTraceViewItem(_rootItem);
    item->_lastmsg = msg;

    CanDbMessage *dbmsg = backend()->findDbMessage(msg);
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
    AggregatedTraceViewItem *item = _map.value(makeUniqueKey(msg));
    if (item) {
        item->_prevmsg = item->_lastmsg;
        item->_lastmsg = msg;
    }
}

void AggregatedTraceViewModel::onUpdateModel()
{

    if (!_pendingMessageInserts.isEmpty()) {
        beginInsertRows(QModelIndex(), _rootItem->childCount(), _rootItem->childCount()+_pendingMessageInserts.size()-1);
        foreach (CanMessage msg, _pendingMessageInserts) {
            createItem(msg);
        }
        endInsertRows();
        _pendingMessageInserts.clear();
    }

    if (!_pendingMessageUpdates.isEmpty()) {
        QSet<int> updatedRows;
        foreach (CanMessage msg, _pendingMessageUpdates) {
            AggregatedTraceViewItem *item = _map.value(makeUniqueKey(msg));
            if (item) {
                updateItem(msg);
                updatedRows.insert(item->row());
            }
        }
        
        foreach (int r, updatedRows) {
            AggregatedTraceViewItem *item = _rootItem->child(r);
            if (item) {
                QModelIndex msgIdx = createIndex(r, 0, item);
                emit dataChanged(msgIdx, msgIdx.sibling(r, column_count - 1));

                if (item->childCount() > 0) {
                    QModelIndex firstChild = index(0, 0, msgIdx);
                    QModelIndex lastChild = index(item->childCount() - 1, column_count - 1, msgIdx);
                    emit dataChanged(firstChild, lastChild);
                }
            }
        }
        _pendingMessageUpdates.clear();
    }

}

void AggregatedTraceViewModel::onSetupChanged()
{
    beginResetModel();
    for (AggregatedTraceViewItem *item : _map.values()) {
        item->removeChildren();
        CanDbMessage *dbmsg = backend()->findDbMessage(item->_lastmsg);
        if (dbmsg) {
            for (int i=0; i<dbmsg->getSignals().length(); i++) {
                item->appendChild(new AggregatedTraceViewItem(item));
            }
        }
    }
    endResetModel();
}

void AggregatedTraceViewModel::beforeAppend(int num_messages)
{
    CanTrace *trace = backend()->getTrace();
    int start_id = trace->size();

    for (int i=start_id; i<start_id + num_messages; i++) {
        CanMessage msg = trace->getMessage(i);
        unique_key_t key = makeUniqueKey(msg);
        if (_map.contains(key) || _pendingMessageInserts.contains(key)) {
            _pendingMessageUpdates.append(msg);
        } else {
            _pendingMessageInserts[key] = msg;
        }
    }

    onUpdateModel();
}

void AggregatedTraceViewModel::beforeClear()
{
    beginResetModel();
    delete _rootItem;
    _map.clear();
    _rootItem = new AggregatedTraceViewItem(0);
}

void AggregatedTraceViewModel::afterClear()
{
    endResetModel();
}

AggregatedTraceViewModel::unique_key_t AggregatedTraceViewModel::makeUniqueKey(const CanMessage &msg) const
{
    return ((uint64_t)msg.getInterfaceId() << 32) | msg.getRawId();
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

CanMessage AggregatedTraceViewModel::getMessage(const QModelIndex &index) const
{
    if (!index.isValid()) return CanMessage();
    AggregatedTraceViewItem *item = static_cast<AggregatedTraceViewItem*>(index.internalPointer());
    if (item == _rootItem) return CanMessage();
    return (item->parent() == _rootItem) ? item->_lastmsg : item->parent()->_lastmsg;
}

QVariant AggregatedTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    AggregatedTraceViewItem *item = (AggregatedTraceViewItem *)index.internalPointer();
    if (!item) { return QVariant(); }

    if (index.column() == column_index) {
        return (item->parent() == _rootItem) ? QVariant((uint32_t)(index.row() + 1)) : QVariant();
    }

    if (item->parent() == _rootItem) { // CanMessage row
        return data_DisplayRole_Message(index, role, item->_lastmsg, item->_prevmsg);
    } else { // CanSignal Row
        return data_DisplayRole_Signal(index, role, item->parent()->_lastmsg);
    }
}

QVariant AggregatedTraceViewModel::data_TextColorRole(const QModelIndex &index, int role) const
{
    (void) role;
    AggregatedTraceViewItem *item = (AggregatedTraceViewItem *)index.internalPointer();
    if (!item) { return QVariant(); }

    if (item->parent() == _rootItem) { // CanMessage row
        return QVariant();
    } else { // CanSignal Row
        return data_TextColorRole_Signal(index, role, item->parent()->_lastmsg);
    }
}


