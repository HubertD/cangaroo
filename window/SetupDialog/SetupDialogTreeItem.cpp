#include "SetupDialogTreeItem.h"
#include <QModelIndex>
#include <drivers/CanInterfaceProvider.h>
#include "SetupDialogTreeModel.h"

SetupDialogTreeItem::SetupDialogTreeItem(item_type type, SetupDialogTreeItem *parent)
  : _type(type), _parent(parent), setup(0), network(0), intf(0), candb(0)
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
            return intf->getName();
        case SetupDialogTreeModel::column_driver:
            return intf->getProvider()->getName();
        case SetupDialogTreeModel::column_bitrate:
            return intf->getBitrate();
        default:
            return QVariant();
    }
}

QVariant SetupDialogTreeItem::dataCanDb(const QModelIndex &index) const
{
    switch (index.column()) {
        case SetupDialogTreeModel::column_device:
            return candb->getBaseFilename();
        case SetupDialogTreeModel::column_filename:
            return candb->getFilename();
        case SetupDialogTreeModel::column_path:
            return candb->getFilename();
        default:
            return QVariant();
    }
}

QVariant SetupDialogTreeItem::dataDisplayRole(const QModelIndex &index) const
{
    switch (_type) {
        case type_root: return "Setup";
        case type_network: return network->name();
        case type_interface_root: return "Interfaces";
        case type_interface: return dataInterface(index);
        case type_candb_root: return "Can Databases";
        case type_candb: return dataCanDb(index);
    }
candb->getBaseFilename();
    return QVariant();
}

SetupDialogTreeItem::item_type SetupDialogTreeItem::getType()
{
    return _type;
}

