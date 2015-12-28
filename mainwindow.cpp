#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QThread>

#include "views/LinearTraceViewModel.h"
#include "views/AggregatedTraceViewModel.h"

#include "drivers/socketcan/SocketCanInterface.h"
#include "drivers/socketcan/SocketCanInterfaceProvider.h"
#include "drivers/CanListener.h"

#include "parser/dbc/DbcParser.h"
#include "model/CanDb.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    DbcParser parser;
    QFile *dbc = new QFile("test.dbc");
    parser.parseFile(dbc, &_candb);

    _trace = new CanTrace(this, 100);

    _linearTraceViewModel = new LinearTraceViewModel(&_candb, _trace);
    _aggregatedTraceViewModel = new AggregatedTraceViewModel(&_candb, _trace);

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
    connect(ui->cbAggregate, SIGNAL(stateChanged(int)), this, SLOT(onCbTraceTypeChanged(int)));

    _provider = new SocketCanInterfaceProvider();
    _provider->update();

    qRegisterMetaType<CanMessage>("CanMessage");

    CanInterfaceList interfaces = _provider->getInterfaceList();
    int i=0;
    for (CanInterfaceList::iterator it=interfaces.begin(); it!=interfaces.end(); ++it) {
        CanInterface *intf = *it;
        intf->open();
        intf->setId(i++);

        QThread* thread = new QThread;
        CanListener *listener = new CanListener(0, intf);
        listener->moveToThread(thread);
        connect(thread, SIGNAL(started()), listener, SLOT(run()));
        connect(listener, SIGNAL(messageReceived(CanMessage)), _trace, SLOT(enqueueMessage(CanMessage)));
        thread->start();
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onCbTraceTypeChanged(int i)
{
    if (i==Qt::Checked) {
        ui->tree->setModel(_aggregatedTraceViewModel);
    } else {
        ui->tree->setModel(_linearTraceViewModel);
    }

}

void MainWindow::rowsInserted(const QModelIndex &parent, int first, int last)
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
