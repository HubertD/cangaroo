#include "TraceWindow.h"
#include "ui_TraceWindow.h"

#include <QDebug>
#include <QDomDocument>
#include <QSortFilterProxyModel>
#include "LinearTraceViewModel.h"
#include "AggregatedTraceViewModel.h"

TraceWindow::TraceWindow(QWidget *parent, Backend &backend) :
    MdiWindow(parent),
    ui(new Ui::TraceWindow),
    _backend(&backend)
{
    ui->setupUi(this);

    _linearTraceViewModel = new LinearTraceViewModel(backend);
    _linearProxyModel = new QSortFilterProxyModel(this);
    _linearProxyModel->setSourceModel(_linearTraceViewModel);
    _linearProxyModel->setDynamicSortFilter(true);

    _aggregatedTraceViewModel = new AggregatedTraceViewModel(backend);
    _aggregatedProxyModel = new QSortFilterProxyModel(this);
    _aggregatedProxyModel->setSourceModel(_aggregatedTraceViewModel);
    _aggregatedProxyModel->setDynamicSortFilter(true);

    setMode(mode_linear);
    setAutoScroll(false);

    ui->tree->setUniformRowHeights(true);
    ui->tree->setColumnWidth(0, 80);
    ui->tree->setColumnWidth(1, 70);
    ui->tree->setColumnWidth(2, 50);
    ui->tree->setColumnWidth(3, 90);
    ui->tree->setColumnWidth(4, 200);
    ui->tree->setColumnWidth(5, 50);
    ui->tree->setColumnWidth(6, 200);
    ui->tree->sortByColumn(BaseTraceViewModel::column_canid);

    connect(_linearTraceViewModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));

}

TraceWindow::~TraceWindow()
{
    delete ui;
    delete _aggregatedTraceViewModel;
    delete _linearTraceViewModel;
}

void TraceWindow::setMode(TraceWindow::mode_t mode)
{
    bool isChanged = (_mode != mode);
    _mode = mode;

    if (_mode==mode_linear) {
        ui->tree->setSortingEnabled(false);
        ui->tree->setModel(_linearTraceViewModel);
    } else {
        ui->tree->setSortingEnabled(true);
        ui->tree->setModel(_aggregatedProxyModel);
    }

    if (isChanged) {
        ui->cbAggregated->setChecked(_mode==mode_aggregated);
        emit(settingsChanged(this));
    }
}

void TraceWindow::setAutoScroll(bool doAutoScroll)
{
    if (doAutoScroll != _doAutoScroll) {
        _doAutoScroll = doAutoScroll;
        ui->cbAutoScroll->setChecked(_doAutoScroll);
        emit(settingsChanged(this));
    }
}

bool TraceWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!MdiWindow::saveXML(backend, xml, root)) {
        return false;
    }

    root.setAttribute("type", "TraceWindow");
    root.setAttribute("mode", (_mode==mode_linear) ? "linear" : "aggregated");

    QDomElement elLinear = xml.createElement("LinearTraceView");
    elLinear.setAttribute("autoscroll", (ui->cbAutoScroll->checkState() == Qt::Checked) ? 1 : 0);
    root.appendChild(elLinear);

    QDomElement elAggregated = xml.createElement("AggregatedTraceView");
    elAggregated.setAttribute("SortColumn", _aggregatedProxyModel->sortColumn());
    root.appendChild(elAggregated);

    return true;
}

bool TraceWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!MdiWindow::loadXML(backend, el)) {
        return false;
    }

    setMode((el.attribute("mode", "linear") == "linear") ? mode_linear : mode_aggregated);

    QDomElement elLinear = el.firstChildElement("LinearTraceView");
    setAutoScroll(elLinear.attribute("autoscroll", "0").toInt() != 0);

    QDomElement elAggregated = el.firstChildElement("AggregatedTraceView");
    int sortColumn = elAggregated.attribute("SortColumn", "-1").toInt();
    ui->tree->sortByColumn(sortColumn);

    return true;
}

void TraceWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;

    if ((_mode==mode_linear) && (ui->cbAutoScroll->checkState() == Qt::Checked)) {
        ui->tree->scrollToBottom();
    }

}

void TraceWindow::on_cbAggregated_stateChanged(int i)
{
    setMode( (i==Qt::Checked) ? mode_aggregated : mode_linear );
}

void TraceWindow::on_cbAutoScroll_stateChanged(int i)
{
    setAutoScroll(i==Qt::Checked);
}
