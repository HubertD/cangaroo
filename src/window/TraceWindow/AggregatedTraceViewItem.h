#ifndef AGGREGATEDTRACEVIEWITEM_H
#define AGGREGATEDTRACEVIEWITEM_H


#include <sys/time.h>
#include <model/CanMessage.h>
#include <QList>

class AggregatedTraceViewItem
{
public:
    AggregatedTraceViewItem(AggregatedTraceViewItem *parent);
    virtual ~AggregatedTraceViewItem();

    void appendChild(AggregatedTraceViewItem *child);
    AggregatedTraceViewItem *child(int row) const;
    int childCount() const;
    int row() const;
    AggregatedTraceViewItem *parent() const;
    AggregatedTraceViewItem *firstChild() const;
    AggregatedTraceViewItem *lastChild() const;

    CanMessage _lastmsg, _prevmsg;

private:
    AggregatedTraceViewItem *_parent;
    QList<AggregatedTraceViewItem *> _children;

};


#endif // AGGREGATEDTRACEVIEWITEM_H
