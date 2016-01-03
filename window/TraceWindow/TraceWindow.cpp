#include "TraceWindow.h"
#include "ui_TraceWindow.h"
#include "LinearTraceViewModel.h"
#include "AggregatedTraceViewModel.h"

TraceWindow::TraceWindow(QWidget *parent, CanTrace *trace) :
    QWidget(parent),
    ui(new Ui::TraceWindow),
    _trace(trace)
{
    ui->setupUi(this);

    _linearTraceViewModel = new LinearTraceViewModel(_trace);
    _aggregatedTraceViewModel = new AggregatedTraceViewModel(_trace);

    ui->tree->setModel(_linearTraceViewModel);
    ui->tree->setUniformRowHeights(true);
    ui->tree->setColumnWidth(0, 80);
    ui->tree->setColumnWidth(1, 70);
    ui->tree->setColumnWidth(2, 50);
    ui->tree->setColumnWidth(3, 90);
    ui->tree->setColumnWidth(4, 200);
    ui->tree->setColumnWidth(5, 50);
    ui->tree->setColumnWidth(6, 200);

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
        ui->tree->setModel(_aggregatedTraceViewModel);
    } else {
        ui->tree->setModel(_linearTraceViewModel);
    }
}

void TraceWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;

    if (ui->cbAutoScroll->checkState() == Qt::Checked) {
        if (ui->tree->model()==_linearTraceViewModel) {
            ui->tree->scrollToBottom();
        }
    }

}
