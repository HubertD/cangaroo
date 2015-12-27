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

    trace = new CanTrace(this, 100);

    //model = new LinearTraceViewModel(trace);
    ui->tree->setModel(new AggregatedTraceViewModel(&_candb, trace));
    ui->tree->setUniformRowHeights(true);

    SocketCanInterfaceProvider prov;
    prov.update();

    qRegisterMetaType<CanMessage>("CanMessage");

    CanInterfaceList interfaces = prov.getInterfaceList();
    int i=0;
    for (CanInterfaceList::iterator it=interfaces.begin(); it!=interfaces.end(); ++it) {
        CanInterface *intf = *it;
        intf->open();
        intf->setId(i++);

        QThread* thread = new QThread;
        CanListener *listener = new CanListener(0, intf);
        listener->moveToThread(thread);
        connect(thread, SIGNAL(started()), listener, SLOT(run()));
        connect(listener, SIGNAL(messageReceived(CanMessage)), trace, SLOT(enqueueMessage(CanMessage)));
        thread->start();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
