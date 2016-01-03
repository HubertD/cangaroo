#ifndef SETUPDIALOGTREEMODEL_H
#define SETUPDIALOGTREEMODEL_H

#include "SetupDialogTreeItem.h"

#include <QAbstractItemModel>
#include <model/MeasurementSetup.h>

class SetupDialogTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit SetupDialogTreeModel(MeasurementSetup *setup, QObject *parent=0);
    virtual ~SetupDialogTreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    void addCanDb(const QModelIndex &parent, CanDb *db);
    void deleteCanDb(const QModelIndex &index);

private:
    SetupDialogTreeItem *_rootItem;
    MeasurementSetup *_setup;

    SetupDialogTreeItem *itemOrRoot(const QModelIndex &index) const;
    void load(MeasurementSetup *setup);
};

#endif // SETUPDIALOGTREEMODEL_H
