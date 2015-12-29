#ifndef TRACEVIEW_H
#define TRACEVIEW_H

#include <QWidget>

namespace Ui {
class TraceView;
}

class MeasurementSetup;
class LinearTraceViewModel;
class AggregatedTraceViewModel;

class TraceView : public QWidget
{
    Q_OBJECT

public:
    explicit TraceView(QWidget *parent, MeasurementSetup *setup);
    ~TraceView();

    MeasurementSetup *getSetup() const;

public slots:
    void onCbTraceTypeChanged(int i);
    void rowsInserted(const QModelIndex & parent, int first, int last);

private:
    Ui::TraceView *ui;
    MeasurementSetup *_setup;
    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
};

#endif // TRACEVIEW_H
