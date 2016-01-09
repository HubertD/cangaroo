#ifndef TRACEWINDOW_H
#define TRACEWINDOW_H

#include <MdiWindow.h>

namespace Ui {
class TraceWindow;
}

class QDomDocument;
class QDomElement;
class QSortFilterProxyModel;
class LinearTraceViewModel;
class AggregatedTraceViewModel;
class Backend;

class TraceWindow : public MdiWindow
{
    Q_OBJECT

public:
    typedef enum mode {
        mode_linear,
        mode_aggregated
    } mode_t;

    explicit TraceWindow(QWidget *parent, Backend &backend);
    ~TraceWindow();

    void setMode(mode_t mode);
    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);

public slots:
    void onCbTraceTypeChanged(int i);
    void rowsInserted(const QModelIndex & parent, int first, int last);

private:
    Ui::TraceWindow *ui;
    Backend *_backend;
    mode_t _mode;

    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
    QSortFilterProxyModel *_aggregatedProxyModel;
    QSortFilterProxyModel *_linearProxyModel;
};

#endif // TRACEWINDOW_H
