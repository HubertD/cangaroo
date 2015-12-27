#include "AggregatedTraceViewItem.h"

AggregatedTraceViewItem::AggregatedTraceViewItem(AggregatedTraceViewItem *parent)
  : _parent(parent)
{
    _interval.tv_sec = 0;
    _interval.tv_usec = 0;
}

AggregatedTraceViewItem::~AggregatedTraceViewItem()
{
    qDeleteAll(_children);
}

void AggregatedTraceViewItem::appendChild(AggregatedTraceViewItem *child)
{
    _children.append(child);
}

AggregatedTraceViewItem *AggregatedTraceViewItem::child(int row) const
{
    return _children.value(row);
}

int AggregatedTraceViewItem::childCount() const
{
    return _children.count();
}

int AggregatedTraceViewItem::row() const
{
    if (_parent) {
        return _parent->_children.indexOf(const_cast<AggregatedTraceViewItem*>(this));
    } else {
        return 0;
    }
}

AggregatedTraceViewItem *AggregatedTraceViewItem::parent() const
{
    return _parent;
}
