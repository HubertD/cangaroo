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

#pragma once

#include <QAbstractItemModel>
#include "TraceViewTypes.h"

class Backend;
class CanTrace;
class CanMessage;
class CanDbSignal;

class BaseTraceViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        column_index,
        column_timestamp,
        column_channel,
        column_direction,
        column_type,
        column_canid,
        column_sender,
        column_name,
        column_dlc,
        column_data,
        column_comment,
        column_count
    };

public:
    BaseTraceViewModel(Backend &backend);
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    virtual CanMessage getMessage(const QModelIndex &index) const = 0;

    Backend *backend() const;
    CanTrace *trace() const;

    timestamp_mode_t timestampMode() const;
    void setTimestampMode(timestamp_mode_t timestampMode);

protected:
    virtual QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    virtual QVariant data_DisplayRole_Message(const QModelIndex &index, int role, const CanMessage &currentMsg, const CanMessage &lastMsg) const;
    virtual QVariant data_DisplayRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const;
    virtual QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;
    virtual QVariant data_TextColorRole(const QModelIndex &index, int role) const;
    virtual QVariant data_TextColorRole_Signal(const QModelIndex &index, int role, const CanMessage &msg) const;

    QVariant formatTimestamp(timestamp_mode_t mode, const CanMessage &currentMsg, const CanMessage &lastMsg) const;

private:
    Backend *_backend;
    timestamp_mode_t _timestampMode;

};
