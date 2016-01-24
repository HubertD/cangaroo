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

#pragma once

#include <QAbstractItemModel>
#include <QDateTime>
#include <QList>
#include <core/Backend.h>

class LogItem {
public:
    QDateTime dt;
    log_level_t level;
    QString text;
};

class LogModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        column_time,
        column_level,
        column_text,
        column_count
    };

public:
    LogModel(Backend &backend);
    virtual ~LogModel();

    void clear();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
    void onLogMessage(const QDateTime dt, const log_level_t level, const QString msg);

private:
    QList<LogItem*> _items;

    static QString logLevelText(log_level_t level);
};
