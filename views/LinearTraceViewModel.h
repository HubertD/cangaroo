#ifndef LINEARTRACEVIEWMODEL_H
#define LINEARTRACEVIEWMODEL_H

#include <QAbstractItemModel>
#include <setup/MeasurementSetup.h>
#include <model/CanDb.h>
#include <model/CanTrace.h>
#include "BaseTraceViewModel.h"

class LinearTraceViewModel : public BaseTraceViewModel
{
    Q_OBJECT

public:
    LinearTraceViewModel(MeasurementSetup *setup);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;

public slots:
    void beforeAppend(int num_messages);
    void afterAppend(int num_messages);

private:
    virtual QVariant data_DisplayRole(const QModelIndex &index, int role) const;
};

#endif // LINEARTRACEVIEWMODEL_H
