#ifndef AGGREGATEDTRACEVIEWMODEL_H
#define AGGREGATEDTRACEVIEWMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QList>
#include <QTimer>

#include <sys/time.h>

#include <views/BaseTraceViewModel.h>
#include <model/CanMessage.h>
#include <drivers/CanInterface.h>

#include "AggregatedTraceViewItem.h"


class CanTrace;

class AggregatedTraceViewModel : public BaseTraceViewModel
{
    Q_OBJECT

public:
    typedef uint64_t unique_key_t;
    typedef QMap<unique_key_t, AggregatedTraceViewItem*> CanIdMap;

public:
    AggregatedTraceViewModel(CanTrace *trace);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;

public slots:
    void messageReceived(const CanMessage &msg);
    void onUpdateTimer();

private:
    CanIdMap _map;
    AggregatedTraceViewItem *_rootItem;
    QTimer *_updateTimer;
    QList<CanMessage> _pendingMessageUpdates;
    QList<CanMessage> _pendingMessageInserts;

    unique_key_t makeUniqueKey(const CanMessage &msg);
    void createItem(const CanMessage &msg, AggregatedTraceViewItem *item, unique_key_t key);
    double getTimeDiff(const timeval t1, const timeval t2) const;
    
protected:
    virtual QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    virtual QVariant data_TextColorRole(const QModelIndex &index, int role) const;

private slots:
    void createItem(const CanMessage &msg);
    void updateItem(const CanMessage &msg);
};

#endif // AGGREGATEDTRACEVIEWMODEL_H
