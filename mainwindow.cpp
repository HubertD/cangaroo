#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <QStringList>
#include <QStandardItem>
#include <QTimer>

#include "views/CanMessageTraceViewModel.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model = new CanMessageTraceViewModel();
    ui->tree->setModel(model);
    ui->tree->setUniformRowHeights(true);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(appendMessages()));
    timer->start(200);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendMessages()
{
    model->addMessages(200);
    //ui->tree->scrollToBottom();
}
