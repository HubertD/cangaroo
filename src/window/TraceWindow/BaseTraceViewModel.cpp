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
#include "qtooltip.h"

#include <QDateTime>
#include <QColor>

#include <core/Backend.h>
#include <core/CanTrace.h>
#include <core/CanMessage.h>
#include <core/CanDbMessage.h>
#include <core/ThemeManager.h>
#include <iostream>

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
                case column_index:
                return QString(tr("Index"));
                case column_timestamp:
                    return QString(tr("Timestamp"));
                case column_channel:
                    return QString(tr("Channel"));
                case column_direction:
                    return QString(tr("RX/TX"));
                case column_type:
                    return QString(tr("Type"));
                case column_canid:
                    return QString("ID");
                case column_sender:
                    return QString(tr("Sender"));
                case column_name:
                    return QString(tr("Name"));
                case column_dlc:
                    return QString("DLC");
                case column_data:
                    return QString(tr("Data"));
                case column_comment:
                    return QString(tr("Comment"));
            }
        }

    }
    else if (role == Qt::TextAlignmentRole) {
        switch (section) {
            case column_index: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_timestamp: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_channel: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_direction: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_type: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_canid: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_sender: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
            case column_name: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
            case column_dlc: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
            case column_data: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
            case column_comment: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
            default: return QVariant();
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
        case Qt::ForegroundRole:
            return data_TextColorRole(index, role);
        case Qt::ToolTipRole:
        {
            QString data = index.data(Qt::DisplayRole).toString();
            uint  length = data.length();
            if(length>24)
            {
                uint div = length / 24;
                for(uint i = 0;i< div-1;i++)
                {
                    if((i+1)%2 == 0 || index.column() != column_data)
                        data.insert(24*(i+1)+i,"\n");
                    else
                        data.insert(24*(i+1)+i," ");
                }
            }
            return data;
        }
            //return QString("Row%1, Column%2").arg(row + 1).arg(col +1);
        default:
            return QVariant();
    }
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

    if (mode==timestamp_mode_delta) {

        double t_current = currentMsg.getFloatTimestamp();
        double t_last = lastMsg.getFloatTimestamp();
        if (t_last==0) {
            return QString().asprintf("%.04lf", 0.00);
        } else {
            return QString().asprintf("%.04lf", t_current-t_last);
        }

    } else if (mode==timestamp_mode_absolute) {

        return currentMsg.getDateTime().toString("hh:mm:ss.zzz");

    } else if (mode==timestamp_mode_relative) {

        double t_current = currentMsg.getFloatTimestamp();
        return QString().asprintf("%.04lf", t_current - backend()->getTimestampAtMeasurementStart());

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

        case column_index:
            return index.internalId();

        case column_timestamp:
            return formatTimestamp(_timestampMode, currentMsg, lastMsg);

        case column_channel:
            return backend()->getInterfaceName(currentMsg.getInterfaceId());

        case column_direction:
            return currentMsg.isRX()?"RX":"TX";

        case column_type:
        {
            QString _type = QString(currentMsg.isFD()? "FD.":"") + QString(currentMsg.isExtended()? "EXT." : "STD.") + QString(currentMsg.isRTR()?"RTR":"") + QString((currentMsg.isBRS()?"BRS":""));
            return _type;
        }

        case column_canid:
            return currentMsg.getIdString();

        case column_name:
            return (dbmsg) ? dbmsg->getName() : "";

        case column_sender:
            return (dbmsg) ? dbmsg->getSender()->name() : "";

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
    uint64_t raw_data;
    QString value_name;
    QString unit;

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
                unit = dbsignal->getUnit();
                if (unit.isEmpty()) {
                    return dbsignal->convertRawValueToPhysical(raw_data);
                } else {
                    return QString("%1 %2").arg(dbsignal->convertRawValueToPhysical(raw_data)).arg(unit);
                }
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
        case column_index: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
        case column_timestamp: return static_cast<int>(Qt::AlignRight) + static_cast<int>(Qt::AlignVCenter);
        case column_channel: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
        case column_direction: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
        case column_type: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
        case column_canid: return static_cast<int>(Qt::AlignRight) + static_cast<int>(Qt::AlignVCenter);
        case column_sender: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
        case column_name: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
        case column_dlc: return static_cast<int>(Qt::AlignCenter) + static_cast<int>(Qt::AlignVCenter);
        case column_data: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
        case column_comment: return static_cast<int>(Qt::AlignLeft) + static_cast<int>(Qt::AlignVCenter);
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
        bool isDark = ThemeManager::instance().isDarkMode();
        return QVariant::fromValue(isDark ? QColor(100, 100, 100) : QColor(200, 200, 200));
    }
}

