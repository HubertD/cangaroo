#include "LinearTraceViewModel.h"
#include <iostream>
#include <stddef.h>
#include <Backend.h>

LinearTraceViewModel::LinearTraceViewModel(Backend &backend)
  : BaseTraceViewModel(backend)
{
    connect(backend.getTrace(), SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect(backend.getTrace(), SIGNAL(afterAppend()), this, SLOT(afterAppend()));
    connect(backend.getTrace(), SIGNAL(beforeClear()), this, SLOT(beforeClear()));
    connect(backend.getTrace(), SIGNAL(afterClear()), this, SLOT(afterClear()));
}

QModelIndex LinearTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalId()) {
        return createIndex(row, column, (unsigned int)(0x80000000 | parent.internalId()));
    } else {
        return createIndex(row, column, row+1);
    }
}

QModelIndex LinearTraceViewModel::parent(const QModelIndex &child) const
{
    (void) child;
    quintptr id = child.internalId();
    if (id & 0x80000000) {
        return createIndex(id & 0x7FFFFFFF, 0, (unsigned int)(id & 0x7FFFFFFF));
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
            const CanMessage *msg = trace()->getMessage(id-1);
            if (msg) {
                CanDbMessage *dbmsg = backend()->findDbMessage(*msg);
                return (dbmsg!=0) ? dbmsg->getSignals().length() : 0;
            } else {
                return 0;
            }
        }
    } else {
        return trace()->size();
    }
}

int LinearTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

bool LinearTraceViewModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent)>0;
}

void LinearTraceViewModel::beforeAppend(int num_messages)
{
    beginInsertRows(QModelIndex(), trace()->size(), trace()->size()+num_messages-1);
}

void LinearTraceViewModel::afterAppend()
{
    endInsertRows();
}

void LinearTraceViewModel::beforeClear()
{
    beginResetModel();
}

void LinearTraceViewModel::afterClear()
{
    endResetModel();
}

QVariant LinearTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    quintptr id = index.internalId();
    int msg_id = (id & ~0x80000000)-1;

    const CanMessage *msg = trace()->getMessage(msg_id);
    if (!msg) { return QVariant(); }

    if (id & 0x80000000) {
        return data_DisplayRole_Signal(index, role, msg);
    } else if (id) {
        if (msg_id>1) {
            const CanMessage *prev_msg = trace()->getMessage(msg_id-1);
            return data_DisplayRole_Message(index, role, *msg, *prev_msg);
        } else {
            return data_DisplayRole_Message(index, role, *msg, CanMessage());
        }
    }

    return QVariant();
}
