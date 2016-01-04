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

    QIcon icon(":/assets/icon.png");
    setWindowIcon(icon);

    QImage bgimg(":/assets/mdibg.png");
    ui->mdiArea->setBackground(bgimg);
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceView()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(createLogView()));
    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(showSetupDialog()));

    connect(ui->actionStart_Measurement, SIGNAL(triggered()), this, SLOT(startMeasurement()));
    connect(ui->actionStop_Measurement, SIGNAL(triggered()), this, SLOT(stopMeasurement()));
    ui->actionStop_Measurement->setEnabled(false);

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));


    _setup = createDefaultSetup();
    _trace = new CanTrace(this, _setup, 100);

    QMdiSubWindow *logView = createLogView();
    logView->setGeometry(0, 500, 1000, 200);

    QMdiSubWindow *traceViewWindow = createTraceView();
    traceViewWindow->setGeometry(0, 0, 1000, 500);


    /*QMdiSubWindow *graphViewWindow = createGraphView();
    graphViewWindow->setGeometry(0, 500, 1000, 200);
*/
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
    return createSubWindow(new TraceWindow(ui->mdiArea, _trace));
}

QMdiSubWindow *MainWindow::createLogView()
{
    return createSubWindow(new LogView(ui->mdiArea, _logger));
}

QMdiSubWindow *MainWindow::createGraphView()
{
    return createSubWindow(new GraphView(ui->mdiArea, _setup));
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

bool MainWindow::showSetupDialog()
{
    SetupDialog dlg(0);
    MeasurementSetup *new_setup = dlg.showSetupDialog(*_setup);
    if (new_setup) {
        MeasurementSetup *old_setup = _setup;
        _setup = new_setup;
        _trace->setSetup(_setup);
        delete old_setup;
        return true;
    } else {
        return false;
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "About cangaroo", "cangaroo version 0.1\n(c)2015-2016 Hubert Denkmair");
}

void MainWindow::startMeasurement()
{
    if (!showSetupDialog()) {
        return;
    }

    qRegisterMetaType<CanMessage>("CanMessage");

    qDebug("starting measurement");
    ui->actionStart_Measurement->setEnabled(false);

    int i=0;
    foreach (MeasurementNetwork *network, _setup->getNetworks()) {
        i++;
        foreach (pCanInterface intf, network->_canInterfaces) {
            intf->setId(i);
            intf->open();

            qDebug() << "listening on interface" << intf->getName();

            CanListener *listener = new CanListener(0, intf);
            connect(listener, SIGNAL(messageReceived(CanMessage)), _trace, SLOT(enqueueMessage(CanMessage)));
            listener->startThread();
            _listeners.append(listener);
        }
    }

    ui->actionStop_Measurement->setEnabled(true);
}

void MainWindow::stopMeasurement()
{
    ui->actionStop_Measurement->setEnabled(false);

    foreach (CanListener *listener, _listeners) {
        listener->requestStop();
    }

    foreach (CanListener *listener, _listeners) {
        listener->waitFinish();
        qDebug() << "closing interface" << listener->getInterface()->getName();
        listener->getInterface()->close();
    }

    qDeleteAll(_listeners);
    _listeners.clear();

    qDebug("measurement stopped");
    ui->actionStart_Measurement->setEnabled(true);
}

MeasurementSetup *MainWindow::createDefaultSetup()
{

    CanInterfaceProvider *provider = new SocketCanInterfaceProvider();
    provider->update();

    MeasurementSetup *defaultSetup = new MeasurementSetup(this);
    int i = 1;
    foreach (pCanInterface intf, provider->getInterfaceList()) {
        MeasurementNetwork *network = defaultSetup->createNetwork();
        network->setName(QString().sprintf("Network %d", i++));
        network->addCanInterface(intf);
    }

    return defaultSetup;
}
