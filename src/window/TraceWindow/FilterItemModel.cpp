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

FilterItemModel::FilterItemModel()
{
    root = createObjectNode(0, node_root);
    addFilterSet("Filterset 1");
    addFilterSet("Filterset 2");
}

QModelIndex FilterItemModel::rootIndex()
{
    return createIndex(0, 0, root);
}

QObject *FilterItemModel::createObjectNode(QObject *parent, FilterItemModel::node_type_t type)
{
    QObject *retval = new QObject(parent);
    retval->setProperty("type", type);
    return retval;
}

FilterItemModel::node_type_t FilterItemModel::getNodeType(const QObject *obj) const
{
    if (obj) {
        return (node_type_t)obj->property("type").toInt();
    } else {
        return node_unknown;
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
        return 1;
    }
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
        return createIndex(row, column, root);
    }
}

QModelIndex FilterItemModel::parent(const QModelIndex &index) const
{
    QObject *obj = (QObject*) index.internalPointer();
    if (!index.isValid() || (obj==root) || (!obj)) {
        return QModelIndex();
    }

    QObject *p = obj->parent();
    if (!p) {
        return QModelIndex();
    } else if (p==root) {
        return createIndex(0, 0, p);
    } else {
        return createIndex(p->parent()->children().indexOf(p), 0, p);
    }
}

Qt::ItemFlags FilterItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    QObject *obj = (QObject*) index.internalPointer();

    switch (getNodeType(obj)) {
        case node_accept_list:
        case node_drop_list:
            flags |= Qt::ItemIsDropEnabled;
            break;
        case node_filter_item:
            flags |= Qt::ItemIsDragEnabled;
            break;
        default:
            break;
    }

    return flags;
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

        switch (getNodeType(obj)) {
            case node_root:
                return "root";
            case node_filter_set:
                return obj->property("title");
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

QObject *FilterItemModel::addFilterSet(QString title)
{
    QObject *fs = createObjectNode(root, node_filter_set);
    fs->setProperty("title", title);
    createObjectNode(fs, node_accept_list);
    createObjectNode(fs, node_drop_list);
    return fs;
}

