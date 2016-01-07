#include "TraceWindow.h"
#include "ui_TraceWindow.h"
#include "LinearTraceViewModel.h"
#include "AggregatedTraceViewModel.h"

TraceWindow::TraceWindow(QWidget *parent, Backend &backend) :
    QWidget(parent),
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


    ui->tree->setModel(_linearTraceViewModel);
    ui->tree->setUniformRowHeights(true);
    ui->tree->setColumnWidth(0, 80);
    ui->tree->setColumnWidth(1, 70);
    ui->tree->setColumnWidth(2, 50);
    ui->tree->setColumnWidth(3, 90);
    ui->tree->setColumnWidth(4, 200);
    ui->tree->setColumnWidth(5, 50);
    ui->tree->setColumnWidth(6, 200);

    ui->tree->setSortingEnabled(true);

    connect(_linearTraceViewModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(ui->cbAggregated, SIGNAL(stateChanged(int)), this, SLOT(onCbTraceTypeChanged(int)));

}

TraceWindow::~TraceWindow()
{
    delete ui;
    delete _aggregatedTraceViewModel;
    delete _linearTraceViewModel;
}

void TraceWindow::onCbTraceTypeChanged(int i)
{
    if (i==Qt::Checked) {
        ui->tree->setModel(_aggregatedProxyModel);
    } else {
        ui->tree->setModel(_linearProxyModel);
    }
}

void TraceWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;

    if (ui->cbAutoScroll->checkState() == Qt::Checked) {
        if (ui->tree->model()==_linearProxyModel) {
            ui->tree->scrollToBottom();
        }
    }

}
