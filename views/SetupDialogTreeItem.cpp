#include "SetupDialogTreeItem.h"

SetupDialogTreeItem::SetupDialogTreeItem(item_type type, SetupDialogTreeItem *parent)
  : _type(type), _parent(parent)
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

QVariant SetupDialogTreeItem::dataDisplayRole() const
{
    switch (_type) {
        case type_root: return "Setup";
        case type_network: return network->name();
        case type_interface_root: return "Interfaces";
        case type_interface: return intf->getName();
        case type_candb_root: return "Can Databases";
        case type_candb: return candb->getFilename();
    }

    return QVariant();
}

SetupDialogTreeItem::item_type SetupDialogTreeItem::getType()
{
    return _type;
}

