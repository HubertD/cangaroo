#include "GraphView.h"
#include "ui_GraphView.h"

GraphView::GraphView(QWidget *parent, MeasurementSetup *setup) :
    QWidget(parent),
    ui(new Ui::GraphView),
    _setup(setup)
{
    ui->setupUi(this);
}

GraphView::~GraphView()
{
    delete ui;
}
