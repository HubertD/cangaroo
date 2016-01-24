/*

  Copyright (c) 2016 Hubert Denkmair <hubert@denkmair.de>

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


#include "LogModel.h"

LogModel::LogModel(Backend &backend)
{
    connect(&backend, SIGNAL(onLogMessage(QDateTime,log_level_t,QString)), this, SLOT(onLogMessage(QDateTime,log_level_t,QString)));
}

LogModel::~LogModel()
{
    qDeleteAll(_items);
    _items.clear();
}

void LogModel::clear()
{
    beginResetModel();
    _items.clear();
    endResetModel();
}

QModelIndex LogModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QModelIndex();
    } else {
        return createIndex(row, column, (quintptr)0);
    }
}

QModelIndex LogModel::parent(const QModelIndex &child) const
{
    (void) child;
    return QModelIndex();
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    (void) parent;
    return _items.size();
}

int LogModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

bool LogModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {

        if (orientation == Qt::Horizontal) {
            switch (section) {
                case column_time:
                    return QString("Time");
                case column_level:
                    return QString("Level");
                case column_text:
                    return QString("Message");
            }
        }

    }
    return QVariant();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{

    if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
            case column_time:
                return Qt::AlignRight + Qt::AlignVCenter;
            case column_level:
                return Qt::AlignCenter + Qt::AlignVCenter;
            case column_text:
                return Qt::AlignLeft + Qt::AlignVCenter;
            default:
                return QVariant();
        }
    }

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (!index.isValid()) {
        return QVariant();
    }

    LogItem *item = _items.value(index.row(), 0);
    if (item) {

        switch (index.column()) {
            case column_time:
                return item->dt.toString("hh:mm:ss");
            case column_level:
                return logLevelText(item->level);
            case column_text:
                return item->text;
            default:
                return QVariant();
        }

    }
    return QVariant();
}

void LogModel::onLogMessage(const QDateTime dt, const log_level_t level, const QString msg)
{
    LogItem *item = new LogItem();
    item->dt = dt;
    item->level = level;
    item->text = msg;

    beginInsertRows(QModelIndex(), _items.size(), _items.size());
    _items.append(item);
    endInsertRows();
}

QString LogModel::logLevelText(log_level_t level)
{
    switch (level) {
        case log_level_debug: return "debug";
        case log_level_info: return "info";
        case log_level_warning: return "warning";
        case log_level_error: return "error";
        case log_level_critical: return "critical";
        case log_level_fatal: return "fatal";
        default: return "";
    }
}

