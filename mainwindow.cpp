#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QMdiArea>
#include <QSignalMapper>
#include <QCloseEvent>
#include <QDebug>
#include <QDomDocument>

#include "Logger.h"
#include <model/MeasurementSetup.h>
#include <window/TraceWindow/TraceWindow.h>
#include <window/SetupDialog/SetupDialog.h>
#include <window/LogWindow/LogWindow.h>
#include <window/GraphWindow/GraphWindow.h>

MainWindow::MainWindow(Logger *logger, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _logger(logger)
{
    ui->setupUi(this);
    _baseWindowTitle = windowTitle();

    QIcon icon(":/assets/icon.png");
    setWindowIcon(icon);

    QImage bgimg(":/assets/mdibg.png");
    ui->mdiArea->setBackground(bgimg);
    ui->mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceWindow()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(createLogWindow()));
    connect(ui->actionGraph_View, SIGNAL(triggered()), this, SLOT(createGraphWindow()));
    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(showSetupDialog()));

    connect(ui->actionStart_Measurement, SIGNAL(triggered()), this, SLOT(startMeasurement()));
    connect(ui->actionStop_Measurement, SIGNAL(triggered()), this, SLOT(stopMeasurement()));

    connect(&backend, SIGNAL(beginMeasurement()), this, SLOT(updateMeasurementActions()));
    connect(&backend, SIGNAL(endMeasurement()), this, SLOT(updateMeasurementActions()));
    updateMeasurementActions();

    connect(ui->actionSave_Trace_to_file, SIGNAL(triggered(bool)), this, SLOT(saveTraceToFile()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    backend.addCanDriver(&_socketcan);

    setWorkspaceModified(false);
    newWorkspace();

    /*QMdiSubWindow *graphViewWindow = createGraphView();
    graphViewWindow->setGeometry(0, 500, 1000, 200);
*/
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMeasurementActions()
{
    bool running = backend.isMeasurementRunning();
    ui->actionStart_Measurement->setEnabled(!running);
    ui->actionStop_Measurement->setEnabled(running);
}

void MainWindow::on_workspace_modified()
{
    setWorkspaceModified(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (askSaveBecauseWorkspaceModified()!=QMessageBox::Cancel) {
        event->accept();
    } else {
        event->ignore();
    }
}

QMdiSubWindow *MainWindow::createSubWindow(MdiWindow *window)
{
    QMdiSubWindow *retval = ui->mdiArea->addSubWindow(window);
    connect(window, SIGNAL(settingsChanged(MdiWindow*)), this, SLOT(on_workspace_modified()));
    window->show();
    return retval;
}

void MainWindow::stopAndClearMeasurement()
{
    backend.stopMeasurement();
    QCoreApplication::processEvents();
    backend.clearTrace();
}

bool MainWindow::loadWorkspaceWindow(QDomElement el)
{
    MdiWindow *window;
    QString type = el.attribute("type");
    if (type=="LogWindow") {
        window = new LogWindow(ui->mdiArea, *_logger);
    } else if (type=="TraceWindow") {
        window = new TraceWindow(ui->mdiArea, backend);
    } else if (type=="GraphWindow") {
        window = new GraphWindow(ui->mdiArea, backend);
    } else {
        qCritical() << "cannot construct window of unknown type" << type;
        window = new MdiWindow(ui->mdiArea);
    }

    window->loadXML(backend, el);
    QMdiSubWindow *mdi = createSubWindow(window);
    mdi->setGeometry(
        el.attribute("left", "0").toInt(),
        el.attribute("top", "0").toInt(),
        el.attribute("width", "100").toInt(),
        el.attribute("height", "100").toInt()
    );

    return true;
}

bool MainWindow::loadWorkspaceSetup(QDomElement el)
{
    MeasurementSetup setup(&backend);
    if (setup.loadXML(backend, el)) {
        backend.setSetup(setup);
        return true;
    } else {
        return false;
    }
}

void MainWindow::loadWorkspaceFromFile(QString filename)
{

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open workspace settings file:" << filename;
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        qCritical() << "Cannot load settings from" << filename;
        return;
    }
    file.close();

    stopAndClearMeasurement();
    ui->mdiArea->closeAllSubWindows();

    QDomElement windowsRoot = doc.firstChild().firstChildElement("windows");
    QDomNodeList windows = windowsRoot.elementsByTagName("window");
    for (int i=0; i<windows.length(); i++) {
        if (!loadWorkspaceWindow(windows.item(i).toElement())) {
            qDebug() << "Could not read window " << i << "settings from" << filename;
            continue;
        }
    }

    QDomElement setupRoot = doc.firstChild().firstChildElement("setup");
    if (loadWorkspaceSetup(setupRoot)) {
        _workspaceFileName = filename;
        setWorkspaceModified(false);
    } else {
        qCritical() << "Unable to read measurement setup from workspace config file" << filename;
    }
}

bool MainWindow::saveWorkspaceToFile(QString filename)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("cangaroo-workspace");
    doc.appendChild(root);

    QDomElement windowsRoot = doc.createElement("windows");
    root.appendChild(windowsRoot);

    foreach (QMdiSubWindow *window, ui->mdiArea->subWindowList()) {
        QDomElement wnode = doc.createElement("window");
        wnode.setAttribute("left", window->geometry().left());
        wnode.setAttribute("top", window->geometry().top());
        wnode.setAttribute("width", window->geometry().width());
        wnode.setAttribute("height", window->geometry().height());

        MdiWindow *mdiwin = dynamic_cast<MdiWindow*>(window->widget());
        if (!mdiwin->saveXML(backend, doc, wnode)) {
            qCritical() << "Cannot save window settings to file";
            return false;
        }

        windowsRoot.appendChild(wnode);
    }

    QDomElement setupRoot = doc.createElement("setup");
    if (!backend.getSetup().saveXML(backend, doc, setupRoot)) {
        qCritical() << "Cannot save measurement setup to file";
        return false;
    }
    root.appendChild(setupRoot);

    QFile outFile(filename);
    if(outFile.open(QIODevice::WriteOnly|QIODevice::Text) ) {
        QTextStream stream( &outFile );
        stream << doc.toString();
        outFile.close();
        _workspaceFileName = filename;
        setWorkspaceModified(false);
        qDebug() << "saved workspace settings to " << filename;
        return true;
    } else {
        qCritical() << "Cannot open workspace file for writing:" << filename;
        return false;
    }

}

void MainWindow::newWorkspace()
{
    if (askSaveBecauseWorkspaceModified() != QMessageBox::Cancel) {
        stopAndClearMeasurement();
        _workspaceFileName.clear();
        setWorkspaceModified(false);
        ui->mdiArea->closeAllSubWindows();
        createLogWindow()->setGeometry(0, 500, 1000, 200);
        createTraceWindow()->setGeometry(0, 0, 1000, 500);
        backend.setDefaultSetup();
    }
}

void MainWindow::loadWorkspace()
{
    if (askSaveBecauseWorkspaceModified() != QMessageBox::Cancel) {
        QString filename = QFileDialog::getOpenFileName(this, "Open workspace configuration", "", "Workspace config files (*.cangaroo)");
        if (!filename.isNull()) {
            loadWorkspaceFromFile(filename);
        }
    }
}

bool MainWindow::saveWorkspace()
{
    if (_workspaceFileName.isEmpty()) {
        return saveWorkspaceAs();
    } else {
        return saveWorkspaceToFile(_workspaceFileName);
    }
}

bool MainWindow::saveWorkspaceAs()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save workspace configuration", "", "Workspace config files (*.cangaroo)");
    if (!filename.isNull()) {
        return saveWorkspaceToFile(filename);
    } else {
        return false;
    }
}

void MainWindow::setWorkspaceModified(bool modified)
{
    _workspaceModified = modified;

    QString title = _baseWindowTitle;
    if (!_workspaceFileName.isEmpty()) {
        QFileInfo fi(_workspaceFileName);
        title += " - " + fi.fileName();
    }
    if (_workspaceModified) {
        title += '*';
    }
    setWindowTitle(title);
}

int MainWindow::askSaveBecauseWorkspaceModified()
{
    if (_workspaceModified) {
        QMessageBox msgBox;
        msgBox.setText("The current workspace has been modified.");
        msgBox.setInformativeText("Do you want to save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int result = msgBox.exec();

        if (result == QMessageBox::Save) {
            if (saveWorkspace()) {
                return QMessageBox::Save;
            } else {
                return QMessageBox::Cancel;
            }
        }

        return result;
    } else {
        return QMessageBox::Discard;
    }
}

QMdiSubWindow *MainWindow::createTraceWindow() {
    return createSubWindow(new TraceWindow(ui->mdiArea, backend));
}

QMdiSubWindow *MainWindow::createLogWindow()
{
    return createSubWindow(new LogWindow(ui->mdiArea, *_logger));
}

QMdiSubWindow *MainWindow::createGraphWindow()
{
    return createSubWindow(new GraphWindow(ui->mdiArea, backend));
}

void MainWindow::setActiveSubWindow(QWidget *window) {
    if (window) {
        ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
    }
}

bool MainWindow::showSetupDialog()
{
    MeasurementSetup new_setup(&backend);
    new_setup.cloneFrom(backend.getSetup());

    SetupDialog dlg(backend, 0);
    if (dlg.showSetupDialog(new_setup)) {
        backend.setSetup(new_setup);
        setWorkspaceModified(true);
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
        backend.startMeasurement();
    }
}

void MainWindow::stopMeasurement()
{
    backend.stopMeasurement();
}

void MainWindow::saveTraceToFile()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save Trace to file", "", "Candump Files (*.candump)");
    if (!filename.isNull()) {
        backend.saveCanDump(filename);
    }
}


void MainWindow::on_action_TraceClear_triggered()
{
    backend.clearTrace();
}

void MainWindow::on_action_WorkspaceSave_triggered()
{
    saveWorkspace();
}

void MainWindow::on_action_WorkspaceSaveAs_triggered()
{
    saveWorkspaceAs();
}

void MainWindow::on_action_WorkspaceOpen_triggered()
{
    loadWorkspace();
}

void MainWindow::on_action_WorkspaceNew_triggered()
{
    newWorkspace();
}
