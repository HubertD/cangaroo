#include "SetupDialogTreeModel.h"

SetupDialogTreeModel::SetupDialogTreeModel(QObject *parent)
  : QAbstractItemModel(parent),
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
    if (parent.column() <= 0) {
        return itemOrRoot(parent)->getChildCount();
    } else {
        return 0;
    }
}

int SetupDialogTreeModel::columnCount(const QModelIndex &parent) const
{
    (void) parent;
    return column_count;
}

void SetupDialogTreeModel::addCanDb(const QModelIndex &parent, pCanDb db)
{
    SetupDialogTreeItem *parentItem = static_cast<SetupDialogTreeItem*>(parent.internalPointer());
    if (!parentItem) { return; }

    SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_candb, parentItem);
    if (!item) { return; }

    item->candb = db;

    if (parentItem->network) {
        parentItem->network->addCanDb(db);
        beginInsertRows(parent, rowCount(parent), rowCount(parent));
        parentItem->appendChild(item);
        endInsertRows();
    }
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

void SetupDialogTreeModel::addInterface(const QModelIndex &parent, pCanInterface interface)
{
    SetupDialogTreeItem *parentItem = static_cast<SetupDialogTreeItem*>(parent.internalPointer());
    if (!parentItem) { return; }
    if (parentItem && parentItem->network) {
        beginInsertRows(parent, parentItem->getChildCount(), parentItem->getChildCount());

        SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_interface, parentItem);
        item->intf = interface;
        parentItem->appendChild(item);

        parentItem->network->addCanInterface(interface);

        endInsertRows();
    }
}

void SetupDialogTreeModel::deleteInterface(const QModelIndex &index)
{
    SetupDialogTreeItem *item = static_cast<SetupDialogTreeItem*>(index.internalPointer());
    if (!item) { return; }

    SetupDialogTreeItem *parentItem = item->getParentItem();
    if (parentItem && parentItem->network && parentItem->network->_canInterfaces.contains(item->intf)) {
        parentItem->network->_canInterfaces.removeAll(item->intf);
        beginRemoveRows(index.parent(), item->row(), item->row());
        item->getParentItem()->removeChild(item);
        endRemoveRows();
    }
}

SetupDialogTreeItem *SetupDialogTreeModel::itemOrRoot(const QModelIndex &index) const
{
    return index.isValid() ? static_cast<SetupDialogTreeItem*>(index.internalPointer()) : _rootItem;
}

void SetupDialogTreeModel::load(MeasurementSetup *setup)
{
    _rootItem = new SetupDialogTreeItem(SetupDialogTreeItem::type_root, 0);
    _rootItem->setup = setup;

    foreach (MeasurementNetwork *network, setup->getNetworks()) {
        SetupDialogTreeItem *item_network = new SetupDialogTreeItem(SetupDialogTreeItem::type_network, _rootItem);
        item_network->network = network;

        SetupDialogTreeItem *item_intf_root = new SetupDialogTreeItem(SetupDialogTreeItem::type_interface_root, item_network);
        item_intf_root->network = network;
        item_network->appendChild(item_intf_root);

        foreach (pCanInterface intf, network->_canInterfaces) {
            SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_interface, item_intf_root);
            item->intf = intf;
            item_intf_root->appendChild(item);
        }

        SetupDialogTreeItem *item_candb_root = new SetupDialogTreeItem(SetupDialogTreeItem::type_candb_root, item_network);
        item_candb_root->network = network;
        item_network->appendChild(item_candb_root);

        foreach (pCanDb candb, network->_canDbs) {
            SetupDialogTreeItem *item = new SetupDialogTreeItem(SetupDialogTreeItem::type_candb, item_candb_root);
            item->candb = candb;
            item_candb_root->appendChild(item);
        }

        _rootItem->appendChild(item_network);
    }

}

