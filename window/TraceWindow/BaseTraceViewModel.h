#ifndef BASETRACEVIEWMODEL_H
#define BASETRACEVIEWMODEL_H

#include <QAbstractItemModel>

class Backend;
class CanTrace;
class CanMessage;

class BaseTraceViewModel : public QAbstractItemModel
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

public:
    BaseTraceViewModel(Backend &backend);
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    Backend *backend() const;
    CanTrace *trace() const;


protected:
    Backend *_backend;

    virtual QVariant data_DisplayRole(const QModelIndex &index, int role) const;
    virtual QVariant data_DisplayRole_Message(const QModelIndex &index, int role, const CanMessage *msg, struct timeval tv) const;
    virtual QVariant data_DisplayRole_Signal(const QModelIndex &index, int role, const CanMessage *msg) const;
    virtual QVariant data_TextAlignmentRole(const QModelIndex &index, int role) const;
    virtual QVariant data_TextColorRole(const QModelIndex &index, int role) const;

};

#endif // BASETRACEVIEWMODEL_H
