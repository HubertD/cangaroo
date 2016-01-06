#ifndef SETUPDIALOGTREEITEM_H
#define SETUPDIALOGTREEITEM_H

#include <QList>
#include <QVariant>

#include <model/MeasurementSetup.h>
#include <model/MeasurementNetwork.h>
#include <model/CanDb.h>
#include <drivers/CanInterface.h>

class SetupDialogTreeItem
{
public:

    typedef enum {
        type_root,
        type_network,
        type_interface_root,
        type_interface,
        type_candb_root,
        type_candb
    } item_type;

public:
    SetupDialogTreeItem(item_type type, SetupDialogTreeItem *parent=0);
    virtual ~SetupDialogTreeItem();

    MeasurementSetup *setup;
    MeasurementNetwork *network;
    pCanInterface intf;
    pCanDb candb;

    SetupDialogTreeItem *getParentItem() const;
    int getChildCount() const;
    void appendChild(SetupDialogTreeItem *child);
    void removeChild(SetupDialogTreeItem *child);

    SetupDialogTreeItem *child(int row) const;
    int row() const;

    QVariant dataDisplayRole(const QModelIndex &index) const;
    item_type getType();

private:
    item_type _type;
    SetupDialogTreeItem *_parent;
    QList<SetupDialogTreeItem*> _children;

    QVariant dataInterface(const QModelIndex &index) const;
    QVariant dataCanDb(const QModelIndex &index) const;
};

#endif // SETUPDIALOGTREEITEM_H
