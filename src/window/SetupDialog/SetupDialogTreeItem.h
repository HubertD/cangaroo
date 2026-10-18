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

#pragma once

#include <QList>
#include <QVariant>

#include <QObject>

#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>
#include <core/CanDb.h>

class SetupDialogTreeItem
{
public:

    typedef enum {
        type_root,
        type_network,
        type_interface_root,
        type_interface,
        type_candb_root,
        type_candb,
    } item_type;

public:
    SetupDialogTreeItem(item_type type, Backend *backend, SetupDialogTreeItem *parent=0);
    virtual ~SetupDialogTreeItem();

    MeasurementSetup *setup;
    MeasurementNetwork *network;
    MeasurementInterface *intf;
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
    Backend *_backend;
    item_type _type;
    SetupDialogTreeItem *_parent;
    QList<SetupDialogTreeItem*> _children;

    QVariant dataInterface(const QModelIndex &index) const;
    QVariant dataCanDb(const QModelIndex &index) const;
};
