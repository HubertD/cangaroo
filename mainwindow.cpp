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

    trace = new CanTrace(this, 500);
    appendMessages();
    appendMessages();
    appendMessages();
    appendMessages();
    appendMessages();
    trace->flushQueue();

    model = new CanMessageTraceViewModel(trace);
    ui->tree->setModel(model);
    ui->tree->setUniformRowHeights(true);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(appendMessages()));
    timer->start(5);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::appendMessages()
{
    CanMessage msg(0x123);
    msg.setExtended(true);
    msg.setLength( 8 );
    msg.setData(0,1,2,3,4,5,6,7);
    trace->enqueueMessage(msg);
    //model->addMessages(200);
    //ui->tree->scrollToBottom();
}
