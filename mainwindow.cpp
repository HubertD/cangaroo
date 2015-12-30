#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QMdiArea>
#include <QSignalMapper>
#include <QCloseEvent>

#include <drivers/socketcan/SocketCanInterface.h>
#include <drivers/socketcan/SocketCanInterfaceProvider.h>
#include <setup/MeasurementSetup.h>
#include <setup/MeasurementNetwork.h>

#include <parser/dbc/DbcParser.h>
#include <model/CanDb.h>

#include <views/TraceView.h>
#include <views/LogView.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceView()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(createLogView()));


    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    startup();

    createTraceView();
    createLogView();
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

QWidget *MainWindow::createSubWindow(QWidget *window)
{
    ui->mdiArea->addSubWindow(window);
    window->show();
    return window;
}


TraceView *MainWindow::createTraceView() {
    return (TraceView*)createSubWindow(new TraceView(ui->mdiArea, setup));
}

LogView *MainWindow::createLogView()
{
    return (LogView*)createSubWindow(new LogView(ui->mdiArea, setup));
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

void MainWindow::startup()
{
    DbcParser parser;
    QFile *dbc = new QFile("test.dbc");
    CanDb *candb = new CanDb();
    parser.parseFile(dbc, candb);

    _provider = new SocketCanInterfaceProvider();
    _provider->update();

    CanInterface *intf;
    MeasurementNetwork *network;
    setup = new MeasurementSetup(this);
    foreach (intf, _provider->getInterfaceList()) {
        network = setup->createNetwork();
        network->addCanInterface(intf);
        network->addCanDb(candb);
    }

    setup->startMeasurement();
}
