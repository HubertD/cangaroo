#ifndef TRACEWINDOW_H
#define TRACEWINDOW_H

#include <QWidget>

namespace Ui {
class TraceWindow;
}

class CanTrace;
class LinearTraceViewModel;
class AggregatedTraceViewModel;

class TraceWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TraceWindow(QWidget *parent, CanTrace *trace);
    ~TraceWindow();

public slots:
    void onCbTraceTypeChanged(int i);
    void rowsInserted(const QModelIndex & parent, int first, int last);

private:
    Ui::TraceWindow *ui;
    CanTrace *_trace;
    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
};

#endif // TRACEWINDOW_H
