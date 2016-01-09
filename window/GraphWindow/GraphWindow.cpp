#include "GraphWindow.h"
#include "ui_GraphWindow.h"

GraphWindow::GraphWindow(QWidget *parent, Backend &backend) :
    MdiWindow(parent),
    ui(new Ui::GraphWindow),
    _backend(&backend)
{
    ui->setupUi(this);
}

GraphWindow::~GraphWindow()
{
    delete ui;
}
