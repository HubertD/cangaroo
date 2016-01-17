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

#include "SetupDialogTreeModel.h"

SetupDialogTreeModel::SetupDialogTreeModel(Backend *backend, QObject *parent)
  : QAbstractItemModel(parent),
    _backend(backend),
    _rootItem(0)
{
}

SetupDialogTreeModel::~SetupDialogTreeModel()
{
    if (_rootItem) {
        delete _rootItem;
    }
}

QVariant SetupDialogTreeModel::data(const QModelIndex &index, int role) const
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());

    if (item) {

        if (role==Qt::DisplayRole) {
            return item->dataDisplayRole(index);
        }

    }

    return QVariant();
}

QVariant SetupDialogTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    (void) orientation;

    if (role==Qt::DisplayRole) {
        switch (section) {
            case column_device: return "Device";
            case column_driver: return "Driver";
            case column_bitrate: return "Bitrate";
            case column_filename: return "Filename";
            case column_path: return "Path";
            default: return "";
        }
    } else {
        return QVariant();
    }
}

QModelIndex SetupDialogTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    } else {
        SetupDialogTreeItem *parentItem = itemOrRoot(parent);
        SetupDialogTreeItem *childItem = parentItem->child(row);
        return childItem ? createIndex(row, column, childItem) : QModelIndex();
    }
}

QModelIndex SetupDialogTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) { return QModelIndex(); }

    SetupDialogTreeItem *childItem = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    SetupDialogTreeItem *parentItem = childItem->getParentItem();

    return (parentItem == _rootItem) ? QModelIndex() : createIndex(parentItem->row(), 0, parentItem);
}

int SetupDialogTreeModel::rowCount(const QModelIndex &parent) const
{
    int retval = 0;
    if (parent.column() <= 0) {
        SetupDialogTreeItem *item = itemOrRoot(parent);
        if (item) {
            retval = item->getChildCount();
        }
    }
    return retval;
}

int SetupDialogTreeModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

QModelIndex SetupDialogTreeModel::indexOfItem(const SetupDialogTreeItem *item) const
{
    return createIndex(item->row(), 0, (void*)item);
}

SetupDialogTreeItem *SetupDialogTreeModel::addNetwork()
{
    QString s;
    int i=0;
    while (true) {
        s = QString("Network %1").arg(++i);
        if (_rootItem->setup->getNetworkByName(s)==0) {
            break;
        }
    }

    beginInsertRows(QModelIndex(), _rootItem->getChildCount(), _rootItem->getChildCount());
    MeasurementNetwork *network = _rootItem->setup->createNetwork();
    network->setName(s);
    SetupDialogTreeItem *item = loadNetwork(_rootItem, *network);
    endInsertRows();

    return item;
}

void SetupDialogTreeModel::deleteNetwork(const QModelIndex &index)
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    beginRemoveRows(index.parent(), index.row(), index.row());
    _rootItem->removeChild(item);
    _rootItem->setup->removeNetwork(item->network);
    endRemoveRows();
}

SetupDialogTreeItem *SetupDialogTreeModel::addCanDb(const QModelIndex &parent, pCanDb db)
{
    SetupDialogTreeItem *parentItem = static_cast<SetupDialogTreeItem*>(parent.internalPointer());
    if (!parentItem) { return 0; }

    SetupDialogTreeItem *item = 0;
    if (parentItem->network) {
        beginInsertRows(parent, rowCount(parent), rowCount(parent));
        parentItem->network->addCanDb(db);
        item = loadCanDb(*parentItem, db);
        endInsertRows();
    }
    return item;
}

void SetupDialogTreeModel::deleteCanDb(const QModelIndex &index)
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    if (!item) { return; }

    SetupDialogTreeItem *parentItem = item->getParentItem();
    if (parentItem && parentItem->network && parentItem->network->_canDbs.contains(item->candb)) {
        parentItem->network->_canDbs.removeAll(item->candb);
        beginRemoveRows(index.parent(), item->row(), item->row());
        item->getParentItem()->removeChild(item);
        endRemoveRows();
    }
}

SetupDialogTreeItem *SetupDialogTreeModel::addInterface(const QModelIndex &parent, CanInterfaceId &interface)
{
    SetupDialogTreeItem *parentItem = static_cast<SetupDialogTreeItem*>(parent.internalPointer());
    if (!parentItem) { return 0; }

    SetupDialogTreeItem *item = 0;
    if (parentItem && parentItem->network) {
        beginInsertRows(parent, parentItem->getChildCount(), parentItem->getChildCount());
        MeasurementInterface *mi = parentItem->network->addCanInterface(interface);
        item = loadMeasurementInterface(*parentItem, mi);
        endInsertRows();
    }
    return item;
}

void SetupDialogTreeModel::deleteInterface(const QModelIndex &index)
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    if (!item) { return; }

    SetupDialogTreeItem *parentItem = item->getParentItem();
    if (parentItem && parentItem->network && parentItem->network->interfaces().contains(item->intf)) {
        parentItem->network->removeInterface(item->intf);
        beginRemoveRows(index.parent(), item->row(), item->row());
        item->getParentItem()->removeChild(item);
        endRemoveRows();
    }
}

SetupDialogTreeItem *SetupDialogTreeModel::itemOrRoot(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<SetupDialogTreeItem*>(index.internalPointer()) : _rootItem;
}

SetupDialogTreeItem *SetupDialogTreeModel::loadMeasurementInterface(SetupDialogTreeItem &parent, MeasurementInterface *intf)
{
    SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_interface, _backend, &parent);
    item->intf = intf;
    parent.appendChild(item);
    return item;
}

SetupDialogTreeItem *SetupDialogTreeModel::loadCanDb(SetupDialogTreeItem &parent, pCanDb &db)
{
    SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_candb, _backend, &parent);
    item->candb = db;
    parent.appendChild(item);
    return item;
}

SetupDialogTreeItem *SetupDialogTreeModel::loadNetwork(SetupDialogTreeItem *root, MeasurementNetwork &network)
{
    SetupDialogTreeItem *item_network = new SetupDialogTreeItem(SetupDialogTreeItem::type_network, _backend, root);
    item_network->network = &network;

    SetupDialogTreeItem *item_intf_root = new SetupDialogTreeItem(SetupDialogTreeItem::type_interface_root, _backend, item_network);
    item_intf_root->network = &network;
    item_network->appendChild(item_intf_root);

    SetupDialogTreeItem *item_candb_root = new SetupDialogTreeItem(SetupDialogTreeItem::type_candb_root, _backend, item_network);
    item_candb_root->network = &network;
    item_network->appendChild(item_candb_root);

    foreach (MeasurementInterface *intf, network.interfaces()) {
        loadMeasurementInterface(*item_intf_root, intf);
    }

    foreach (pCanDb candb, network._canDbs) {
        loadCanDb(*item_candb_root, candb);
    }

    root->appendChild(item_network);
    return item_network;
}

void SetupDialogTreeModel::load(MeasurementSetup &setup)
{
    SetupDialogTreeItem *_newRoot = new SetupDialogTreeItem(SetupDialogTreeItem::type_root, 0);
    _newRoot->setup = &setup;

    foreach (MeasurementNetwork *network, setup.getNetworks()) {
        loadNetwork(_newRoot, *network);
    }

    beginResetModel();
    SetupDialogTreeItem *_oldRoot = _rootItem;
    _rootItem = _newRoot;
    delete _oldRoot;
    endResetModel();
}

