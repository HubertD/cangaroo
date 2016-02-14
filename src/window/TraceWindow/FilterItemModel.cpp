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

#include <QMimeData>
#include <QDataStream>
#include <QIODevice>

#include <core/Log.h>
#include <core/CanMessage.h>

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
    if (type==node_filter_set) {
        retval->setProperty("checked", true);
    }
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
        case node_filter_set:
            flags |= Qt::ItemIsUserCheckable;
            break;
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
            case node_filter_item:
                return obj->property("title");
            case node_accept_list:
                return "accept";
            case node_drop_list:
                return "drop";
            default:
                return "unknown";
        }

    } else if (role == Qt::CheckStateRole) {

        QObject *obj = (QObject*) index.internalPointer();
        switch (getNodeType(obj)) {
            case node_filter_set:
                return obj->property("checked").toBool() ? Qt::Checked : Qt::Unchecked;
            default:
                return QVariant();
        }


    } else {
        return QVariant();
    }
}

bool FilterItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        QObject *obj = (QObject*) index.internalPointer();
        bool wasChecked = obj->property("checked").toBool();
        obj->setProperty("checked", !wasChecked);
        return true;
    } else {
        return QAbstractItemModel::setData(index, value, role);
    }
}

QStringList FilterItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/org.cangaroo.can.message";
    types << "application/org.cangaroo.can.message_filter";
    return types;
}

QMimeData *FilterItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid() && (index.column()==0)) {
            QObject *obj = (QObject*) index.internalPointer();

            if (getNodeType(obj) == node_filter_item) {
                CanMessage msg;
                msg.setId(obj->property("can_id").toInt());
                stream << msg;
            }
            mimeData->setProperty("dragIndex", index);
            mimeData->setProperty("dragObject", QVariant::fromValue((void*)obj));
        }
    }
    mimeData->setData("application/org.cangaroo.can.message", encodedData);
    return mimeData;
}

bool FilterItemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (data->hasFormat("application/org.cangaroo.can.message")) {
        return true;
    }

    return false;
}

bool FilterItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    if (action == Qt::IgnoreAction) {
        return true;
    }

    QObject *p = (QObject*) parent.internalPointer();
    if (!p) { return false; }

    if ((getNodeType(p)==node_accept_list) || (getNodeType(p)==node_drop_list)) {

        QVariant dragIndex = data->property("dragIndex");
        if (dragIndex.isValid()) {

            QModelIndex index = dragIndex.toModelIndex();
            QObject *obj = (QObject*) data->property("dragObject").value<void*>();
            if (obj->parent() != p) {
                beginMoveRows(index.parent(), index.row(), index.row(), parent, p->children().count());
                obj->setParent(p);
                endMoveRows();
            }

        } else {

            QByteArray encodedData = data->data("application/org.cangaroo.can.message");
            QDataStream stream(&encodedData, QIODevice::ReadOnly);
            while (!stream.atEnd()) {
                CanMessage msg;
                stream >> msg;

                beginInsertRows(parent, p->children().count(), p->children().count());
                QObject *item = createObjectNode(p, node_filter_item);
                item->setProperty("title", msg.getIdString());
                item->setProperty("can_id", msg.getId());
                endInsertRows();
            }

        }
    }
    return true;
}

QObject *FilterItemModel::addFilterSet(QString title)
{
    QObject *fs = createObjectNode(root, node_filter_set);
    fs->setProperty("title", title);
    createObjectNode(fs, node_accept_list);
    createObjectNode(fs, node_drop_list);
    return fs;
}

