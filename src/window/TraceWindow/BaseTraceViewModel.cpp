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

#include "BaseTraceViewModel.h"

#include <QDateTime>
#include <QColor>

#include <core/Backend.h>
#include <core/CanTrace.h>
#include <core/CanMessage.h>
#include <core/CanDbMessage.h>

BaseTraceViewModel::BaseTraceViewModel(Backend &backend)
{
    _backend = &backend;
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

Qt::ItemFlags BaseTraceViewModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (isMessageRow(index)) {
        flags |= Qt::ItemIsDragEnabled;
    }
    return flags;
}

Backend *BaseTraceViewModel::backend() const
{
    return _backend;
}

CanTrace *BaseTraceViewModel::trace() const
{
    return _backend->getTrace();
}

timestamp_mode_t BaseTraceViewModel::timestampMode() const
{
    return _timestampMode;
}

void BaseTraceViewModel::setTimestampMode(timestamp_mode_t timestampMode)
{
    _timestampMode = timestampMode;
}

QVariant BaseTraceViewModel::formatTimestamp(timestamp_mode_t mode, const CanMessage &currentMsg, const CanMessage &lastMsg) const
{
    double t_current = currentMsg.getFloatTimestamp();

    if (mode==timestamp_mode_delta) {

        double t_last = lastMsg.getFloatTimestamp();
        if (t_last==0) {
            return QVariant();
        } else {
            return QString().sprintf("%.04lf", t_current-t_last);
        }

    } else if (mode==timestamp_mode_absolute) {

        QDateTime dt = QDateTime::fromMSecsSinceEpoch((qint64)(1000*t_current));
        return dt.toString("hh:mm:ss.zzz");

    } else if (mode==timestamp_mode_relative) {

        return QString().sprintf("%.04lf", t_current - backend()->getMeasurementStartTime());

    }

    return QVariant();
}

QVariant BaseTraceViewModel::data_DisplayRole(const QModelIndex &index, int role) const
{
    (void) index;
    (void) role;
    return QVariant();
}

QVariant BaseTraceViewModel::data_DisplayRole_Message(const QModelIndex &index, int role, const CanMessage &currentMsg, const CanMessage &lastMsg) const
{
    (void) role;
    CanDbMessage *dbmsg = backend()->findDbMessage(currentMsg);

    switch (index.column()) {

        case column_timestamp:
            return formatTimestamp(_timestampMode, currentMsg, lastMsg);

        case column_channel:
            return backend()->getInterfaceName(currentMsg.getInterfaceId());

        case column_direction:
            return "rx";

        case column_canid:
            return currentMsg.getIdString();

        case column_name:
            return (dbmsg) ? dbmsg->getName() : "";

        case column_dlc:
            return currentMsg.getLength();

        case column_data:
            return currentMsg.getDataHexString();

        case column_comment:
            return (dbmsg) ? dbmsg->getComment() : "";

        default:
            return QVariant();

    }
}

QVariant BaseTraceViewModel::data_DisplayRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const
{
    (void) role;
    uint32_t raw_data;
    QString value_name;

    CanDbMessage *dbmsg = backend()->findDbMessage(msg);
    if (!dbmsg) { return QVariant(); }

    CanDbSignal *dbsignal = dbmsg->getSignal(index.row());
    if (!dbsignal) { return QVariant(); }

    switch (index.column()) {

        case column_name:
            return dbsignal->name();

        case column_data:

            if (dbsignal->isPresentInMessage(msg)) {
                raw_data = dbsignal->extractRawDataFromMessage(msg);
            } else {
                if (!trace()->getMuxedSignalFromCache(dbsignal, &raw_data)) {
                    return QVariant();
                }
            }

            value_name = dbsignal->getValueName(raw_data);
            if (value_name.isEmpty()) {
                return dbsignal->convertRawValueToPhysical(raw_data);
            } else {
                return QString("%1 - %2").arg(raw_data).arg(value_name);
            }

        case column_comment:
            return dbsignal->comment().replace('\n', ' ');

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

QVariant BaseTraceViewModel::data_TextColorRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const
{
    (void) role;

    CanDbMessage *dbmsg = backend()->findDbMessage(msg);
    if (!dbmsg) { return QVariant(); }

    CanDbSignal *dbsignal = dbmsg->getSignal(index.row());
    if (!dbsignal) { return QVariant(); }

    if (dbsignal->isPresentInMessage(msg)) {
        return QVariant(); // default text color
    } else {
        return QVariant::fromValue(QColor(200,200,200));
    }
}
