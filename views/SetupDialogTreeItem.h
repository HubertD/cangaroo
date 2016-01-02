#ifndef SETUPDIALOGTREEITEM_H
#define SETUPDIALOGTREEITEM_H

#include <QList>
#include <QVariant>

#include <setup/MeasurementSetup.h>
#include <setup/MeasurementNetwork.h>
#include <drivers/CanInterface.h>
#include <model/CanDb.h>

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
    CanInterface *intf;
    CanDb *candb;

    SetupDialogTreeItem *getParentItem() const;
    int getChildCount() const;
    void appendChild(SetupDialogTreeItem *child);
    SetupDialogTreeItem *child(int row) const;
    int row() const;

    QVariant dataDisplayRole() const;
    item_type getType();

private:
    item_type _type;
    SetupDialogTreeItem *_parent;
    QList<SetupDialogTreeItem*> _children;


};

#endif // SETUPDIALOGTREEITEM_H
