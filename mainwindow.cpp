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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createMdiChild()));
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    startup();

    createMdiChild();
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

TraceView *MainWindow::createMdiChild() {
    TraceView *child = new TraceView(ui->mdiArea, setup);
    ui->mdiArea->addSubWindow(child);
    child->show();
    return child;
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
    setup = new MeasurementSetup();
    foreach (intf, _provider->getInterfaceList()) {
        network = setup->createNetwork();
        network->addCanInterface(intf);
        network->addCanDb(candb);
    }

    setup->startMeasurement();
}
