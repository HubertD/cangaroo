#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QThread>

#include "views/CanMessageTraceViewModel.h"
#include "drivers/socketcan/SocketCanInterface.h"
#include "drivers/socketcan/SocketCanInterfaceProvider.h"
#include "drivers/CanListener.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    trace = new CanTrace(this, 100);

    model = new CanMessageTraceViewModel(trace);
    ui->tree->setModel(model);
    ui->tree->setUniformRowHeights(true);

    SocketCanInterfaceProvider prov;
    prov.update();

    qRegisterMetaType<CanMessage>("CanMessage");

    CanInterfaceList interfaces = prov.getInterfaceList();
    for (CanInterfaceList::iterator it=interfaces.begin(); it!=interfaces.end(); ++it) {
        CanInterface *intf = *it;
        intf->open();

        QThread* thread = new QThread;
        CanListener *listener = new CanListener(0, intf);
        listener->moveToThread(thread);
        connect(thread, SIGNAL(started()), listener, SLOT(run()));
        connect(listener, SIGNAL(messageReceived(CanMessage)), trace, SLOT(enqueueMessage(CanMessage)));
        thread->start();
    }


    //QTimer *timer = new QTimer(this);
    //connect(timer, SIGNAL(timeout()), this, SLOT(appendMessages()));
    //timer->start(5);

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
    //ui->tree->scrollToBottom();
}
