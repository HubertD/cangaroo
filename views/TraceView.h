#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QWidget>

namespace Ui {
class TraceView;
}

class CanTrace;
class LinearTraceViewModel;
class AggregatedTraceViewModel;

class TraceView : public QWidget
{
    Q_OBJECT

public:
    explicit TraceView(QWidget *parent, CanTrace *trace);
    ~TraceView();

public slots:
    void onCbTraceTypeChanged(int i);
    void rowsInserted(const QModelIndex & parent, int first, int last);

private:
    Ui::TraceView *ui;
    CanTrace *_trace;
    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
};

#endif // TRACEVIEW_H
