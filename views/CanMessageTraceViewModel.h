#ifndef CANMESSAGEMODEL_H
#define CANMESSAGEMODEL_H

#include <QAbstractItemModel>
#include "../model/CanMessage.h"

typedef std::vector<CanMessage*> CanMessageVector;


class CanMessageTraceViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CanMessageTraceViewModel();

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    bool hasChildren(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void addMessages(int num);
private:
    CanMessageVector _msgs;
    QString can_data_as_hex(CanMessage *msg) const;
    QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;
};

#endif // CANMESSAGEMODEL_H
