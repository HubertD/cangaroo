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

#include "SetupDialogTreeItem.h"
#include <QModelIndex>
#include <driver/CanDriver.h>
#include "SetupDialogTreeModel.h"

SetupDialogTreeItem::SetupDialogTreeItem(item_type type, Backend *backend, SetupDialogTreeItem *parent)
  : setup(0), network(0), intf(0), candb(0), _backend(backend), _type(type), _parent(parent)
{

}

SetupDialogTreeItem::~SetupDialogTreeItem()
{
    qDeleteAll(_children);
}

SetupDialogTreeItem *SetupDialogTreeItem::getParentItem() const
{
    return _parent;
}

int SetupDialogTreeItem::getChildCount() const
{
    return _children.length();
}

void SetupDialogTreeItem::appendChild(SetupDialogTreeItem *child)
{
    _children.append(child);
}

void SetupDialogTreeItem::removeChild(SetupDialogTreeItem *child)
{
    _children.removeAll(child);
}

SetupDialogTreeItem *SetupDialogTreeItem::child(int row) const
{
    return _children.value(row);
}

int SetupDialogTreeItem::row() const
{
    if (_parent) {
        return _parent->_children.indexOf(const_cast<SetupDialogTreeItem*>(this));
    } else {
        return 0;
    }
}

QVariant SetupDialogTreeItem::dataInterface(const QModelIndex &index) const
{
    switch (index.column()) {
        case SetupDialogTreeModel::column_device:
            return _backend->getInterfaceName(intf->canInterface());
        case SetupDialogTreeModel::column_driver:
            return _backend->getDriverName(intf->canInterface());
        case SetupDialogTreeModel::column_bitrate:
            return intf->bitrate();
        default:
            return QVariant();
    }
}

QVariant SetupDialogTreeItem::dataCanDb(const QModelIndex &index) const
{
    switch (index.column()) {
        case SetupDialogTreeModel::column_device:
            return candb->getFileName();
        case SetupDialogTreeModel::column_filename:
            return candb->getFileName();
        case SetupDialogTreeModel::column_path:
            return candb->getDirectory();
        default:
            return QVariant();
    }
}

QVariant SetupDialogTreeItem::dataDisplayRole(const QModelIndex &index) const
{
    switch (_type) {
        case type_root: return "Setup";
        case type_network: return (network!=0) ? network->name() : QVariant();
        case type_interface_root: return "Interfaces";
        case type_interface: return dataInterface(index);
        case type_candb_root: return "Can Databases";
        case type_candb: return dataCanDb(index);
    }
    return QVariant();
}

SetupDialogTreeItem::item_type SetupDialogTreeItem::getType()
{
    return _type;
}

