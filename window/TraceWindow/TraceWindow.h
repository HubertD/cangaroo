#ifndef TRACEWINDOW_H
#define TRACEWINDOW_H

#include <MdiWindow.h>
#include "TraceViewTypes.h"

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
    void setAutoScroll(bool doAutoScroll);
    void setTimestampMode(int mode);

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);

public slots:
    void rowsInserted(const QModelIndex & parent, int first, int last);

private slots:
    void on_cbAggregated_stateChanged(int i);
    void on_cbAutoScroll_stateChanged(int i);

    void on_cbTimestampMode_currentIndexChanged(int index);

private:
    Ui::TraceWindow *ui;
    Backend *_backend;
    mode_t _mode;
    bool _doAutoScroll;
    timestamp_mode_t _timestampMode;

    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;
    QSortFilterProxyModel *_aggregatedProxyModel;
    QSortFilterProxyModel *_linearProxyModel;
};

#endif // TRACEWINDOW_H
