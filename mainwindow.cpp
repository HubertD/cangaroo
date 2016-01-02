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
#include <views/GraphView.h>
#include <views/SetupDialog.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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
    QMdiSubWindow *logView = createLogView();
    QMdiSubWindow *traceViewWindow = createTraceView();
    QMdiSubWindow *graphViewWindow = createGraphView();
    traceViewWindow->setGeometry(0, 0, 1000, 500);
    logView->setGeometry(0, 500, 1000, 200);
    graphViewWindow->setGeometry(0, 500, 1000, 200);

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


QMdiSubWindow *MainWindow::createTraceView() {
    return createSubWindow(new TraceView(ui->mdiArea, setup));
}

QMdiSubWindow *MainWindow::createLogView()
{
    return createSubWindow(new LogView(ui->mdiArea, setup));
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
    int i = 1;
    foreach (intf, _provider->getInterfaceList()) {
        network = setup->createNetwork();
        network->setName(QString().sprintf("Network %d", i++));
        network->addCanInterface(intf);
        network->addCanDb(candb);
    }

    setup->startMeasurement();
}
