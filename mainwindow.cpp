#include "mainwindow.h"

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
    QMainWindow(parent)
{
    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    updateMenus();
    setWindowTitle(tr("MDI"));
    setUnifiedTitleAndToolBarOnMac(true);

    startup();

    createMdiChild();
}

MainWindow::~MainWindow()
{
    delete mdiArea;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    mdiArea->closeAllSubWindows();
    if (mdiArea->currentSubWindow()) {
        event->ignore();
    } else {
        //writeSettings();
        event->accept();
    }

}

TraceView *MainWindow::createMdiChild() {
    TraceView *child = new TraceView(this, setup);
    mdiArea->addSubWindow(child);
    return child;
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

void MainWindow::updateMenus() {

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
