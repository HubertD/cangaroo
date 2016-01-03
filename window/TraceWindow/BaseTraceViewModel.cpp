#include "BaseTraceViewModel.h"

#include <model/CanMessage.h>
#include <model/CanDbMessage.h>

BaseTraceViewModel::BaseTraceViewModel(CanTrace *trace)
  : _trace(trace)
{
}

int BaseTraceViewModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

QVariant BaseTraceViewModel::headerData(int section, Qt::Orientation orientation, int role) const
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


QVariant BaseTraceViewModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
        case Qt::DisplayRole:
            return data_DisplayRole(index, role);
        case Qt::TextAlignmentRole:
            return data_TextAlignmentRole(index, role);
        case Qt::TextColorRole:
            return data_TextColorRole(index, role);
        default:
            return QVariant();
    }
}

CanTrace *BaseTraceViewModel::trace() const
{
    return _trace;
}

QVariant BaseTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;
    return QVariant();
}

QVariant BaseTraceViewModel::data_DisplayRole_Message(const QModelIndex &index, int role, const CanMessage *msg, timeval tv) const
{
    (void) role;
    double intervalD;
    CanDbMessage *dbmsg = trace()->findDbMessage(*msg);

    switch (index.column()) {

        case column_timestamp:
            intervalD = (double)tv.tv_sec + ((double)tv.tv_usec/1000000);
            return (intervalD==0) ? "" : QString().sprintf("%.04f", intervalD);

        case column_channel:
            return _trace->getInterfaceName(*msg->getInterface());

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

        default:
            return QVariant();

    }
}

QVariant BaseTraceViewModel::data_DisplayRole_Signal(const QModelIndex &index, int role, const CanMessage *msg) const
{
    (void) role;
    uint32_t raw_data;
    QString value_name;

    CanDbMessage *dbmsg = _trace->findDbMessage(*msg);
    if (!dbmsg) { return QVariant(); }

    CanDbSignal *dbsignal = dbmsg->getSignal(index.row());
    if (!dbsignal) { return QVariant(); }

    switch (index.column()) {

        case column_name:
            return dbsignal->name();

        case column_data:
            raw_data = msg->extractRawSignal(dbsignal->startBit(), dbsignal->length(), false);
            value_name = dbsignal->getValueName(raw_data);

            if (value_name.isEmpty()) {
                return dbsignal->convertRawValueToPhysical(raw_data);
            } else {
                return QString().number(raw_data) + " - " + value_name;
            }

        case column_comment:
            return dbsignal->comment();

        default:
            return QVariant();

    }
}

QVariant BaseTraceViewModel::data_TextAlignmentRole(const QModelIndex &index, int role) const
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

QVariant BaseTraceViewModel::data_TextColorRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;
    return QVariant();
}
