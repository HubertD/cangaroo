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


#include "FilterItemModel.h"
#include "FilterSet.h"
#include "FilterItem.h"

enum {
    node_root,
    node_filter_set,
    node_accept_list,
    node_drop_list,
    node_filter_item
};

FilterItemModel::FilterItemModel()
{
    FilterSet *set = new FilterSet(&_setup);
    set->setProperty("type", node_filter_set);
    set->acceptList().setProperty("type", node_accept_list);
    set->dropList().setProperty("type", node_drop_list);
}

QModelIndex FilterItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        QObject *obj = (QObject*) parent.internalPointer();
        if (obj) {
            return createIndex(row, column, obj->children().at(row));
        } else {
            return QModelIndex();
        }
    } else {
        return createIndex(row, column, _setup.children().at(row));
    }
}

QModelIndex FilterItemModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        QObject *obj = (QObject*) child.internalPointer();
        QObject *p = obj ? obj->parent() : 0;

        if (p && p->parent()) {
            return createIndex(p->parent()->children().indexOf(p), 0, p);
        }
    }

    return QModelIndex();
}

QVariant FilterItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void) section;
    (void) orientation;
    (void) role;
    return QVariant();
}

QVariant FilterItemModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {

        QObject *obj = (QObject*) index.internalPointer();

        switch (obj->property("type").toInt()) {
            case node_filter_set:
                return "Filterset";
            case node_accept_list:
                return "accept";
            case node_drop_list:
                return "drop";
            case node_filter_item:
                return "condition";
            default:
                return "unknown";
        }
    } else {
        return QVariant();
    }
}

int FilterItemModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return 1;
}

int FilterItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        QObject *obj = (QObject*) parent.internalPointer();
        return obj->children().size();
    } else {
        return _setup.children().count();
    }
}
