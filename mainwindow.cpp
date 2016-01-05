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

    connect(ui->actionSave_Trace_to_file, SIGNAL(triggered(bool)), this, SLOT(saveTraceToFile()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    backend.setSetup(backend.createDefaultSetup());

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
    return createSubWindow(new TraceWindow(ui->mdiArea, backend));
}

QMdiSubWindow *MainWindow::createLogView()
{
    return createSubWindow(new LogView(ui->mdiArea, _logger));
}

QMdiSubWindow *MainWindow::createGraphView()
{
    //return createSubWindow(new GraphView(ui->mdiArea, _setup));
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

bool MainWindow::showSetupDialog()
{
    SetupDialog dlg(0);
    MeasurementSetup *new_setup = dlg.showSetupDialog(*backend.getSetup());
    if (new_setup) {
        backend.setSetup(new_setup);
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
    if (showSetupDialog()) {
        ui->actionStart_Measurement->setEnabled(false);
        if (backend.startMeasurement()) {
            ui->actionStop_Measurement->setEnabled(true);
        } else {
            ui->actionStart_Measurement->setEnabled(true);
        }
    }
}

void MainWindow::stopMeasurement()
{
    ui->actionStop_Measurement->setEnabled(false);
    if (backend.stopMeasurement()) {
        ui->actionStart_Measurement->setEnabled(true);
    } else {
        ui->actionStop_Measurement->setEnabled(true);
    }
}

void MainWindow::saveTraceToFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Trace to file", "", "Candump Files (*.candump)");
    if (!filename.isNull()) {
        backend.saveCanDump(filename);
    }
}
