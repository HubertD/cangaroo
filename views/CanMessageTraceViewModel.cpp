#include "CanMessageTraceViewModel.h"
#include <iostream>
#include <stddef.h>

CanMessageTraceViewModel::CanMessageTraceViewModel(CanTrace *trace)
  : _trace(trace)
{
    connect(_trace, SIGNAL(beforeAppend(int)), this, SLOT(beforeAppend(int)));
    connect(_trace, SIGNAL(afterAppend(int)), this, SLOT(afterAppend(int)));
}

/*
void CanMessageTraceViewModel::addMessages(int num)
{
    beginInsertRows(QModelIndex(), _msgs.size(), _msgs.size()+num-1);
    for (int i=0; i<num; i++) {
        CanMessage *msg = new CanMessage();
        msg->setId(i);
        msg->setExtended(true);
        msg->setLength( i % 9 );
        msg->setByte(0, (i>>24)&0xFF);
        msg->setByte(1, (i>>16)&0xFF);
        msg->setByte(2, (i>> 8)&0xFF);
        msg->setByte(3, (i>> 0)&0xFF);
        msg->setByte(4, (i>>24)&0xFF);
        msg->setByte(5, (i>>16)&0xFF);
        msg->setByte(6, (i>> 8)&0xFF);
        msg->setByte(7, (i>> 0)&0xFF);
        _msgs.push_back(msg);
    }
    endInsertRows();
}
*/

QModelIndex CanMessageTraceViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.internalId()) {
        return createIndex(row, column, 0x80000000 | parent.internalId());
    } else {
        return createIndex(row, column, row+1);
    }
}

QModelIndex CanMessageTraceViewModel::parent(const QModelIndex &child) const
{
    (void) child;
    quintptr id = child.internalId();
    if (id & 0x80000000) {
        return createIndex(id & 0x7FFFFFFF, 0, id & 0x7FFFFFFF);
    }
    return QModelIndex();
}

int CanMessageTraceViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        quintptr id = parent.internalId();
        if (id & 0x80000000) { // node of a message
            return 0;
        } else { // a message
            return _trace->getMessage(id-1)->getLength();
        }
    } else {
        return _trace->size();
    }
}

int CanMessageTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 7;
}

bool CanMessageTraceViewModel::hasChildren(const QModelIndex &parent) const
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


QVariant CanMessageTraceViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case 0:
                    return QString("Timestamp");
                case 1:
                    return QString("Channel");
                case 2:
                    return QString("Rx/Tx");
                case 3:
                    return QString("CAN ID");
                case 4:
                    return QString("Name");
                case 5:
                    return QString("DLC");
                case 6:
                    return QString("Data");
            }
        }

    }
    return QVariant();
}

void CanMessageTraceViewModel::beforeAppend(int num_messages)
{
    beginInsertRows(QModelIndex(), _trace->size(), _trace->size()+num_messages-1);
}

void CanMessageTraceViewModel::afterAppend(int num_messages)
{
    (void) num_messages;
    endInsertRows();
}

QVariant CanMessageTraceViewModel::data(const QModelIndex &index, int role) const
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

QVariant CanMessageTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) role;

    quintptr id = index.internalId();

    if (id & 0x80000000) {
        switch (index.column()) {
            case 3: return QString().sprintf("row %d", index.row());
            case 5: return QString().sprintf("id 0x%08X", index.parent().internalId());
            case 6: return QString().sprintf("id 0x%08X", index.internalId());
        }

    } else {
        const CanMessage *msg = _trace->getMessage(id-1);
        switch (index.column()) {
            case 0: return QString().sprintf("%.04f", ((double)index.row()) / 100);
            case 1: return "vcan0";
            case 2: return (msg->getId() % 7)==0 ? "tx" : "rx";
            case 3: return QString().sprintf("0x%08X", msg->getId());
            case 5: return msg->getLength();
            case 6: return can_data_as_hex(msg);
            //case 6: return QString().sprintf("id 0x%08X", index.internalId());
        }
    }

    return QVariant();
}

QVariant CanMessageTraceViewModel::data_TextAlignmentRole(const QModelIndex &index, int role) const
{
    (void) role;
    switch (index.column()) {
        case 0: return Qt::AlignRight + Qt::AlignVCenter;
        case 1: return Qt::AlignCenter + Qt::AlignVCenter;
        case 2: return Qt::AlignCenter + Qt::AlignVCenter;
        case 3: return Qt::AlignRight + Qt::AlignVCenter;
        case 4: return Qt::AlignLeft + Qt::AlignVCenter;
        case 5: return Qt::AlignCenter + Qt::AlignVCenter;
        case 6: return Qt::AlignLeft + Qt::AlignVCenter;
       default: return QVariant();
    }
}

QString CanMessageTraceViewModel::can_data_as_hex(const CanMessage *msg) const
{
    switch (msg->getLength()) {
        case 0: return "";
        case 1: return QString().sprintf("%02X", msg->getByte(0));
        case 2: return QString().sprintf("%02X %02X", msg->getByte(0), msg->getByte(1));
        case 3: return QString().sprintf("%02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2));
        case 4: return QString().sprintf("%02X %02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2), msg->getByte(3));
        case 5: return QString().sprintf("%02X %02X %02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2), msg->getByte(3), msg->getByte(4));
        case 6: return QString().sprintf("%02X %02X %02X %02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2), msg->getByte(3), msg->getByte(4), msg->getByte(5));
        case 7: return QString().sprintf("%02X %02X %02X %02X %02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2), msg->getByte(3), msg->getByte(4), msg->getByte(5), msg->getByte(6));
        case 8: return QString().sprintf("%02X %02X %02X %02X %02X %02X %02X %02X", msg->getByte(0), msg->getByte(1), msg->getByte(2), msg->getByte(3), msg->getByte(4), msg->getByte(5), msg->getByte(6), msg->getByte(7));
        default: return QString();
    }
}

