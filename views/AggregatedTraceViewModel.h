#ifndef AGGREGATEDTRACEVIEWMODEL_H
#define AGGREGATEDTRACEVIEWMODEL_H

#include <QAbstractItemModel>
#include <map>
#include <vector>

#include <sys/time.h>
#include "model/CanMessage.h"
#include "model/CanTrace.h"


class AggregatedTraceViewItem
{
public:
    CanMessage _lastmsg;
    struct timeval _interval;
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
        column_count
    };

    typedef std::map<uint32_t, AggregatedTraceViewItem*> CanIdMap;
    typedef std::vector<AggregatedTraceViewItem*> ItemList;


public:
    AggregatedTraceViewModel(CanTrace *trace);

    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

public slots:
    void messageReceived(const CanMessage &msg);

private:
    CanTrace *_trace;
    CanIdMap _map;
    ItemList _list;
    QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;

};

#endif // AGGREGATEDTRACEVIEWMODEL_H
