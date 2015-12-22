#ifndef CANMESSAGEMODEL_H
#define CANMESSAGEMODEL_H

#include <QAbstractItemModel>
#include "../model/CanTrace.h"

class CanMessageTraceViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    CanMessageTraceViewModel(CanTrace *trace);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

public slots:
    void beforeAppend(int num_messages);
    void afterAppend(int num_messages);

private:
    CanTrace *_trace;
    QString can_data_as_hex(const CanMessage *msg) const;
    QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;
};

#endif // CANMESSAGEMODEL_H
