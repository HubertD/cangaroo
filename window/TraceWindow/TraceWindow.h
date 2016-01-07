#ifndef TRACEWINDOW_H
#define TRACEWINDOW_H

#include <QWidget>
#include <QSortFilterProxyModel>

namespace Ui {
class TraceWindow;
}

class LinearTraceViewModel;
class AggregatedTraceViewModel;
class Backend;

class TraceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TraceWindow(QWidget *parent, Backend &backend);
    ~TraceWindow();

public slots:
    void onCbTraceTypeChanged(int i);
    void rowsInserted(const QModelIndex & parent, int first, int last);

private:
    Ui::TraceWindow *ui;
    Backend *_backend;
    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
    QSortFilterProxyModel *_aggregatedProxyModel;
    QSortFilterProxyModel *_linearProxyModel;
};

#endif // TRACEWINDOW_H
