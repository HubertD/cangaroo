#ifndef AGGREGATEDTRACEVIEWMODEL_H
#define AGGREGATEDTRACEVIEWMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QList>

#include <sys/time.h>
#include "model/CanMessage.h"
#include "model/CanTrace.h"
#include "model/CanDb.h"

class AggregatedTraceViewItem
{
public:
    AggregatedTraceViewItem(AggregatedTraceViewItem *parent);
    virtual ~AggregatedTraceViewItem();

    void appendChild(AggregatedTraceViewItem *child);
    AggregatedTraceViewItem *child(int row) const;
    int childCount() const;
    int row() const;
    AggregatedTraceViewItem *parent();

    QString _name;
    CanMessage _lastmsg;
    struct timeval _interval;

private:
    AggregatedTraceViewItem *_parent;
    QList<AggregatedTraceViewItem *> _children;


};

class AggregatedTraceViewModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum {
        column_timestamp,
        column_channel,
        column_direction,
        column_canid,
        column_name,
        column_dlc,
        column_data,
        column_comment,
        column_count
    };

    typedef QMap<uint32_t, AggregatedTraceViewItem*> CanIdMap;

public:
    AggregatedTraceViewModel(CanDb *candb, CanTrace *trace);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
    void messageReceived(const CanMessage &msg);

private:
    CanDb *_candb;
    CanTrace *_trace;
    CanIdMap _map;
    AggregatedTraceViewItem *_rootItem;

    QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;

    int getChildCountForItem(const AggregatedTraceViewItem *item) const;

};

#endif // AGGREGATEDTRACEVIEWMODEL_H
