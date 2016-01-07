#ifndef SETUPDIALOGTREEMODEL_H
#define SETUPDIALOGTREEMODEL_H

#include "SetupDialogTreeItem.h"

#include <QAbstractItemModel>
#include <model/MeasurementSetup.h>

class SetupDialogTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        column_device,
        column_driver,
        column_bitrate,
        column_filename,
        column_path,
        column_count
    };

public:
    explicit SetupDialogTreeModel(QObject *parent=0);
    virtual ~SetupDialogTreeModel();

    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QModelIndex indexOfItem(const SetupDialogTreeItem *item) const;

    void load(MeasurementSetup *setup);

    SetupDialogTreeItem *addNetwork();
    void deleteNetwork(const QModelIndex &index);

    SetupDialogTreeItem *addCanDb(const QModelIndex &parent, pCanDb db);
    void deleteCanDb(const QModelIndex &index);

    SetupDialogTreeItem *addInterface(const QModelIndex &parent, pCanInterface interface);
    void deleteInterface(const QModelIndex &index);

private:
    SetupDialogTreeItem *_rootItem;
    SetupDialogTreeItem *itemOrRoot(const QModelIndex &index) const;

    SetupDialogTreeItem *loadNetwork(MeasurementNetwork &network);
    SetupDialogTreeItem *loadCanInterface(SetupDialogTreeItem &parent, pCanInterface &intf);
    SetupDialogTreeItem *loadCanDb(SetupDialogTreeItem &parent, pCanDb &db);
};

#endif // SETUPDIALOGTREEMODEL_H
