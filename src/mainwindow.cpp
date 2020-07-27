/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtWidgets>
#include <QMdiArea>
#include <QSignalMapper>
#include <QCloseEvent>
#include <QDomDocument>

#include <core/MeasurementSetup.h>
#include <core/CanTrace.h>
#include <window/TraceWindow/TraceWindow.h>
#include <window/SetupDialog/SetupDialog.h>
#include <window/LogWindow/LogWindow.h>
#include <window/GraphWindow/GraphWindow.h>
#include <window/CanStatusWindow/CanStatusWindow.h>
#include <window/RawTxWindow/RawTxWindow.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "light_pcapng_ext.h"

#if defined(__linux__)
#include <driver/SocketCanDriver/SocketCanDriver.h>
#else
#include <driver/CandleApiDriver/CandleApiDriver.h>
#endif

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _baseWindowTitle = windowTitle();

    QIcon icon(":/assets/cangaroo.png");
    setWindowIcon(icon);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceWindow()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(addLogWidget()));
    connect(ui->actionGraph_View, SIGNAL(triggered()), this, SLOT(createGraphWindow()));
    connect(ui->actionGraph_View_2, SIGNAL(triggered()), this, SLOT(addGraphWidget()));
    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(showSetupDialog()));
    connect(ui->actionTransmit_View, SIGNAL(triggered()), this, SLOT(addRawTxWidget()));

    connect(ui->actionStart_Measurement, SIGNAL(triggered()), this, SLOT(startMeasurement()));
    connect(ui->actionStop_Measurement, SIGNAL(triggered()), this, SLOT(stopMeasurement()));

    connect(&backend(), SIGNAL(beginMeasurement()), this, SLOT(updateMeasurementActions()));
    connect(&backend(), SIGNAL(endMeasurement()), this, SLOT(updateMeasurementActions()));
    updateMeasurementActions();

    connect(ui->actionSave_Trace_to_file, SIGNAL(triggered(bool)), this, SLOT(saveTraceToFile()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(ui->action_Load_Trace_from_file, SIGNAL(triggered(bool)), this, SLOT(loadTraceFromFile()));

#if defined(__linux__)
    Backend::instance().addCanDriver(*(new SocketCanDriver(Backend::instance())));
#else
    Backend::instance().addCanDriver(*(new CandleApiDriver(Backend::instance())));
#endif

    setWorkspaceModified(false);
    newWorkspace();

    _setupDlg = new SetupDialog(Backend::instance(), 0); // NOTE: must be called after drivers/plugins are initialized

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMeasurementActions()
{
    bool running = backend().isMeasurementRunning();
    ui->actionStart_Measurement->setEnabled(!running);
    ui->actionStop_Measurement->setEnabled(running);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (askSaveBecauseWorkspaceModified()!=QMessageBox::Cancel) {
        backend().stopMeasurement();
        event->accept();
    } else {
        event->ignore();
    }
}

Backend &MainWindow::backend()
{
    return Backend::instance();
}

QMainWindow *MainWindow::createTab(QString title)
{
    QMainWindow *mm = new QMainWindow(this);
    QPalette pal(palette());
    pal.setColor(QPalette::Background, QColor(0xeb, 0xeb, 0xeb));
    mm->setAutoFillBackground(true);
    mm->setPalette(pal);
    ui->mainTabs->addTab(mm, title);
    return mm;
}

QMainWindow *MainWindow::currentTab()
{
    return (QMainWindow*)ui->mainTabs->currentWidget();
}

void MainWindow::stopAndClearMeasurement()
{
    backend().stopMeasurement();
    QCoreApplication::processEvents();
    backend().clearTrace();
    backend().clearLog();
}

void MainWindow::clearWorkspace()
{
    ui->mainTabs->clear();
    _workspaceFileName.clear();
    setWorkspaceModified(false);
}

bool MainWindow::loadWorkspaceTab(QDomElement el)
{
    QMainWindow *mw = 0;
    QString type = el.attribute("type");    
    if (type=="TraceWindow") {
        mw = createTraceWindow(el.attribute("title"));
    } else if (type=="GraphWindow") {
        mw = createGraphWindow(el.attribute("title"));
    } else {
        return false;
    }

    if (mw) {
        ConfigurableWidget *mdi = dynamic_cast<ConfigurableWidget*>(mw->centralWidget());
        if (mdi) {
            mdi->loadXML(backend(), el);
        }
    }

    return true;
}

bool MainWindow::loadWorkspaceSetup(QDomElement el)
{
    MeasurementSetup setup(&backend());
    if (setup.loadXML(backend(), el)) {
        backend().setSetup(setup);
        return true;
    } else {
        return false;
    }
}

void MainWindow::loadWorkspaceFromFile(QString filename)
{

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log_error(QString("Cannot open workspace settings file: %1").arg(filename));
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        log_error(QString("Cannot load settings from file: %1").arg(filename));
        return;
    }
    file.close();

    stopAndClearMeasurement();
    clearWorkspace();

    QDomElement tabsRoot = doc.firstChild().firstChildElement("tabs");
    QDomNodeList tabs = tabsRoot.elementsByTagName("tab");
    for (int i=0; i<tabs.length(); i++) {
        if (!loadWorkspaceTab(tabs.item(i).toElement())) {
            log_warning(QString("Could not read window %1 from file: %2").arg(QString::number(i), filename));
            continue;
        }
    }

    QDomElement setupRoot = doc.firstChild().firstChildElement("setup");
    if (loadWorkspaceSetup(setupRoot)) {
        _workspaceFileName = filename;
        setWorkspaceModified(false);
    } else {
        log_error(QString("Unable to read measurement setup from workspace config file: %1").arg(filename));
    }
}

bool MainWindow::saveWorkspaceToFile(QString filename)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("cangaroo-workspace");
    doc.appendChild(root);

    QDomElement tabsRoot = doc.createElement("tabs");
    root.appendChild(tabsRoot);

    for (int i=0; i < ui->mainTabs->count(); i++) {
        QMainWindow *w = (QMainWindow*)ui->mainTabs->widget(i);

        QDomElement tabEl = doc.createElement("tab");
        tabEl.setAttribute("title", ui->mainTabs->tabText(i));

        ConfigurableWidget *mdi = dynamic_cast<ConfigurableWidget*>(w->centralWidget());
        if (!mdi->saveXML(backend(), doc, tabEl)) {
            log_error(QString("Cannot save window settings to file: %1").arg(filename));
            return false;
        }

        tabsRoot.appendChild(tabEl);
    }

    QDomElement setupRoot = doc.createElement("setup");
    if (!backend().getSetup().saveXML(backend(), doc, setupRoot)) {
        log_error(QString("Cannot save measurement setup to file: %1").arg(filename));
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
        log_info(QString("Saved workspace settings to file: %1").arg(filename));
        return true;
    } else {
        log_error(QString("Cannot open workspace file for writing: %1").arg(filename));
        return false;
    }

}

void MainWindow::newWorkspace()
{
    if (askSaveBecauseWorkspaceModified() != QMessageBox::Cancel) {
        stopAndClearMeasurement();
        clearWorkspace();
        createTraceWindow();
        addRawTxWidget();
        backend().setDefaultSetup();
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

QMainWindow *MainWindow::createTraceWindow(QString title)
{
    if (title.isNull()) {
        title = "Trace";
    }
    QMainWindow *mm = createTab(title);
    mm->setCentralWidget(new TraceWindow(mm, backend()));
    addLogWidget(mm);

    ui->mainTabs->setCurrentWidget(mm);
    return mm;
}

QMainWindow *MainWindow::createGraphWindow(QString title)
{
    if (title.isNull()) {
        title = "Graph";
    }
    QMainWindow *mm = createTab(title);
    mm->setCentralWidget(new GraphWindow(mm, backend()));
    addLogWidget(mm);

    return mm;
}

void MainWindow::addGraphWidget(QMainWindow *parent)
{
    if (!parent) {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget("Graph", parent);
    dock->setWidget(new GraphWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::addRawTxWidget(QMainWindow *parent)
{
    if (!parent) {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget("Transmit View", parent);
    dock->setWidget(new RawTxWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
}


void MainWindow::addLogWidget(QMainWindow *parent)
{
    if (!parent) {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget("Log", parent);
    dock->setWidget(new LogWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::addStatusWidget(QMainWindow *parent)
{
    if (!parent) {
        parent = currentTab();
    }

    QDockWidget *dock = new QDockWidget("CAN Status", parent);
    dock->setWidget(new CanStatusWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

void MainWindow::on_actionCan_Status_View_triggered()
{
    addStatusWidget();
}

bool MainWindow::showSetupDialog()
{
    MeasurementSetup new_setup(&backend());
    new_setup.cloneFrom(backend().getSetup());

    if (_setupDlg->showSetupDialog(new_setup)) {
        backend().setSetup(new_setup);
        setWorkspaceModified(true);
        return true;
    } else {
        return false;
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this,
       "About cangaroo",
       "cangaroo\n"
       "open source can bus analyzer\n"
       "\n"
       "version 0.2.4\n"
       "\n"
       "(c)2015-2017 Hubert Denkmair\n"
       "(c)2018 Ethan Zonca"
    );
}

void MainWindow::startMeasurement()
{
    if (showSetupDialog()) {
        backend().clearTrace();
        backend().startMeasurement();
    }
}

void MainWindow::stopMeasurement()
{
    backend().stopMeasurement();
}

void MainWindow::saveTraceToFile()
{
    QString filters("Vector ASC (*.asc);;Linux candump (*.candump))");
    QString defaultFilter("Vector ASC (*.asc)");

    QFileDialog fileDialog(0, "Save Trace to file", QDir::currentPath(), filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setConfirmOverwrite(true);
    fileDialog.selectNameFilter(defaultFilter);
    fileDialog.setDefaultSuffix("asc");
    if (fileDialog.exec()) {
        QString filename = fileDialog.selectedFiles()[0];
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {

            if (filename.endsWith(".candump", Qt::CaseInsensitive)) {
                backend().getTrace()->saveCanDump(file);
            } else {
                backend().getTrace()->saveVectorAsc(file);
            }

            file.close();
        } else {
            // TODO error message
        }


    }

}


void MainWindow::on_action_TraceClear_triggered()
{
    backend().clearTrace();
    backend().clearLog();
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

void MainWindow::loadTraceFromFile()
{
   QString filters("pcap NG (*.pcapng)");
   QString defaultFilter("pcap NG ASC (*.pcapng)");

   QFileDialog fileDialog(0, "Load Trace from file", QDir::currentPath(), filters);
   fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
   fileDialog.selectNameFilter(defaultFilter);
   fileDialog.setDefaultSuffix("pcap");
   if (fileDialog.exec()) {
       QString QFilename = fileDialog.selectedFiles()[0];
       QByteArray filename = QFilename.toLocal8Bit();
       light_pcapng_t *p = light_pcapng_open_read(filename.data(),LIGHT_FALSE);
       if(p != NULL) {
          int Records = 0;
          QString Msg;
          log_info(QString("opened %1").arg(QFilename));
          for( ; ; ) {
             light_packet_header pkt_header;
             const uint8_t *pkt_data = NULL;
             int res = 0;

             res = light_get_next_packet(p,&pkt_header,&pkt_data);
             if(res == 0) {
                break;
             }

             if (pkt_data != NULL) {
                Records++;
                CanMessage msg;
                struct can_frame *frame = (struct can_frame *) pkt_data;

                msg.setTimestamp(pkt_header.timestamp);
                msg.setRawId(__builtin_bswap32(frame->can_id));
                msg.setInterfaceId(0);

                uint8_t len = frame->can_dlc;
                if(len > 8) {
                   log_warning(QString("DLC reduced from %1 to 8").arg(len));
                   len = 8;
                }

                msg.setLength(len);
                for (int i=0; i<len; i++) {
                    msg.setByte(i, frame->data[i]);
                }
                backend().getTrace()->enqueueMessage(msg, false);
             }
          }
          light_pcapng_close(p);
          log_info(QString("%1 records read").arg(Records));
       }
       else {
          QMessageBox::critical(this,"Error","Couldn't open file");
       }
   }
}
