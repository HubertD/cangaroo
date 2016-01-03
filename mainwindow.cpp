#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QMdiArea>
#include <QSignalMapper>
#include <QCloseEvent>
#include <QDebug>

#include "Logger.h"
#include <drivers/socketcan/SocketCanInterface.h>
#include <drivers/socketcan/SocketCanInterfaceProvider.h>

#include <model/MeasurementSetup.h>
#include <model/MeasurementNetwork.h>
#include <model/CanTrace.h>
#include <model/CanDb.h>
#include <parser/dbc/DbcParser.h>
#include <drivers/CanListener.h>

#include <window/TraceWindow/TraceWindow.h>
#include <views/LogView.h>
#include <views/GraphView.h>
#include <views/SetupDialog.h>

MainWindow::MainWindow(Logger *logger, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _logger(logger)
{
    ui->setupUi(this);
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceView()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(createLogView()));
    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(showSetupDialog()));


    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));


    setup = new MeasurementSetup(this);
    _trace = new CanTrace(this, setup, 100);

    QMdiSubWindow *logView = createLogView();
    logView->setGeometry(0, 500, 1000, 200);

    QMdiSubWindow *traceViewWindow = createTraceView();
    traceViewWindow->setGeometry(0, 0, 1000, 500);

    /*QMdiSubWindow *graphViewWindow = createGraphView();
    graphViewWindow->setGeometry(0, 500, 1000, 200);
*/
    startup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    ui->mdiArea->closeAllSubWindows();
    if (ui->mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        //writeSettings();
        event->accept();
    }

}

QMdiSubWindow *MainWindow::createSubWindow(QWidget *window)
{
    QMdiSubWindow *retval = ui->mdiArea->addSubWindow(window);
    window->show();
    return retval;
}

CanTrace *MainWindow::getTrace()
{
    return _trace;
}


QMdiSubWindow *MainWindow::createTraceView() {
    return createSubWindow(new TraceWindow(ui->mdiArea, _trace));
}

QMdiSubWindow *MainWindow::createLogView()
{
    return createSubWindow(new LogView(ui->mdiArea, _logger));
}

QMdiSubWindow *MainWindow::createGraphView()
{
    return createSubWindow(new GraphView(ui->mdiArea, setup));
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

void MainWindow::showSetupDialog()
{
    SetupDialog dlg(this);
    dlg.showSetupDialog(setup);
}

void MainWindow::startMeasurement(MeasurementSetup *setup)
{
    QThread* thread;
    qRegisterMetaType<CanMessage>("CanMessage");

    qDebug("starting measurement");

    int i=0;
    foreach (MeasurementNetwork *network, setup->getNetworks()) {
        i++;
        foreach (CanInterface *intf, network->_canInterfaces) {
            intf->setId(i);
            intf->open();

            qDebug() << "listening on interface" << intf->getName();

            thread = new QThread;
            CanListener *listener = new CanListener(0, intf);
            listener->moveToThread(thread);
            connect(thread, SIGNAL(started()), listener, SLOT(run()));
            connect(listener, SIGNAL(messageReceived(CanMessage)), _trace, SLOT(enqueueMessage(CanMessage)));
            thread->start();
        }
    }

}

void MainWindow::stopMeasurement()
{
    qDebug("stopping measurement");
}

void MainWindow::startup()
{
    DbcParser parser;
    QFile *dbc = new QFile("test.dbc");
    CanDb *candb = new CanDb();
    parser.parseFile(dbc, candb);

    CanInterfaceProvider *provider = new SocketCanInterfaceProvider();
    provider->update();

    CanInterface *intf;
    MeasurementNetwork *network;
    int i = 1;
    foreach (intf, provider->getInterfaceList()) {
        network = setup->createNetwork();
        network->setName(QString().sprintf("Network %d", i++));
        network->addCanInterface(intf);
        network->addCanDb(candb);
    }

    startMeasurement(setup);
}
