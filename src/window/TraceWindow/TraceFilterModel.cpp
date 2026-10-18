#include "TraceFilterModel.h"
#include "BaseTraceViewModel.h"
#include <QRegularExpression>

TraceFilterModel::TraceFilterModel(QObject *parent)
    : QSortFilterProxyModel{parent},
    _filterText("")
{
   setRecursiveFilteringEnabled(false);
   setDynamicSortFilter(false);
}


void TraceFilterModel::setFilterText(QString filtertext)
{
    _filterText = filtertext;
}

bool TraceFilterModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const
{
    // Pass all on no filter
    if(_filterText.length() == 0)
        return true;

    QModelIndex idx0 = sourceModel()->index(source_row, BaseTraceViewModel::column_channel, source_parent); // Channel
    QModelIndex idx1 = sourceModel()->index(source_row, BaseTraceViewModel::column_canid, source_parent); // CAN ID
    QModelIndex idx2 = sourceModel()->index(source_row, BaseTraceViewModel::column_sender, source_parent); // Sender
    QModelIndex idx3 = sourceModel()->index(source_row, BaseTraceViewModel::column_name, source_parent); // Name
    QModelIndex idx4 = sourceModel()->index(source_row, BaseTraceViewModel::column_type, source_parent); // type

    QString datastr0 = sourceModel()->data(idx0).toString();
    QString datastr1 = sourceModel()->data(idx1).toString();
    QString datastr2 = sourceModel()->data(idx2).toString();
    QString datastr3 = sourceModel()->data(idx3).toString();
    QString datastr4 = sourceModel()->data(idx4).toString();

    QRegularExpression re(_filterText, QRegularExpression::CaseInsensitiveOption);
    if (re.isValid()) {
        if (datastr0.contains(re) ||
            datastr1.contains(re) ||
            datastr2.contains(re) ||
            datastr3.contains(re) ||
            datastr4.contains(re))
            return true;
    } else {
        if (datastr0.contains(_filterText, Qt::CaseInsensitive) ||
            datastr1.contains(_filterText, Qt::CaseInsensitive) ||
            datastr2.contains(_filterText, Qt::CaseInsensitive) ||
            datastr3.contains(_filterText, Qt::CaseInsensitive) ||
            datastr4.contains(_filterText, Qt::CaseInsensitive))
            return true;
    }

    return false;
}
