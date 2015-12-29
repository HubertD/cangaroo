#include "TraceView.h"
#include "ui_TraceView.h"

#include <setup/MeasurementSetup.h>
#include <views/LinearTraceViewModel.h>
#include <views/AggregatedTraceViewModel.h>

TraceView::TraceView(QWidget *parent, MeasurementSetup *setup) :
    QWidget(parent),
    ui(new Ui::TraceView),
    _setup(setup)
{
    ui->setupUi(this);

    _linearTraceViewModel = new LinearTraceViewModel(_setup);
    _aggregatedTraceViewModel = new AggregatedTraceViewModel(_setup);

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

TraceView::~TraceView()
{
    delete ui;
    delete _aggregatedTraceViewModel;
    delete _linearTraceViewModel;
}

MeasurementSetup *TraceView::getSetup() const
{
    return _setup;
}

void TraceView::onCbTraceTypeChanged(int i)
{
    if (i==Qt::Checked) {
        ui->tree->setModel(_aggregatedTraceViewModel);
    } else {
        ui->tree->setModel(_linearTraceViewModel);
    }
}

void TraceView::rowsInserted(const QModelIndex &parent, int first, int last)
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
