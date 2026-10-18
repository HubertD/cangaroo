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
#include "version.h"
#include <QItemSelectionModel>
#include <QMenu>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <QActionGroup>
#include <QDomDocument>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSettings>
#include <QPushButton>
#include <QPalette>
#include <QTimer>
#include <QColorDialog>
#include <QLabel>
#include <QDockWidget>
#include <QStatusBar>
#include <QStyleHints>

#include "core/Backend.h"
#include "core/CanTrace.h"
#include "core/ThemeManager.h"
#include <window/TraceWindow/TraceWindow.h>
#include <window/SetupDialog/SetupDialog.h>
#include <window/LogWindow/LogWindow.h>
#include <window/GraphWindow/GraphWindow.h>
#include <window/CanStatusWindow/CanStatusWindow.h>
#include <window/RawTxWindow/RawTxWindow.h>
#include <window/TxGeneratorWindow/TxGeneratorWindow.h>
#include <window/ReplayWindow/ReplayWindow.h>
#include "window/ScriptWindow/ScriptWindow.h"

#include <driver/SLCANDriver/SLCANDriver.h>
#include <driver/GrIPDriver/GrIPDriver.h>
#include <driver/CANBlastDriver/CANBlasterDriver.h>

#if defined(__linux__)
#include <driver/SocketCanDriver/SocketCanDriver.h>
#else
#include <driver/CandleApiDriver/CandleApiDriver.h>
#endif

namespace {

bool effectiveThemePreferenceIsDark(const QString &preference)
{
    if (preference == QLatin1String("dark")) {
        return true;
    }
    if (preference == QLatin1String("light")) {
        return false;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    switch (qApp->styleHints()->colorScheme()) {
    case Qt::ColorScheme::Dark:
        return true;
    case Qt::ColorScheme::Light:
        return false;
    default:
        break;
    }
#endif
    return qApp->palette().color(QPalette::Window).lightness() < 128;
}

} // namespace

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _baseWindowTitle = windowTitle();

    QLabel* versionLabel = new QLabel(this);
    versionLabel->setText(QString("v%1").arg(CANGAROO_VERSION_STR));
    versionLabel->setStyleSheet("padding-right: 15px; font-weight: bold; font-size: 11px;");
    statusBar()->addPermanentWidget(versionLabel);

    QIcon icon(":/assets/cangaroo.png");
    setWindowIcon(icon);

    connect(ui->action_Trace_View, SIGNAL(triggered()), this, SLOT(createTraceWindow()));
    connect(ui->actionLog_View, SIGNAL(triggered()), this, SLOT(addLogWidget()));
    connect(ui->actionGraph_View, SIGNAL(triggered()), this, SLOT(createGraphWindow()));
    connect(ui->actionGraph_View_2, SIGNAL(triggered()), this, SLOT(addGraphWidget()));
    connect(ui->actionSetup, SIGNAL(triggered()), this, SLOT(showSetupDialog()));
    connect(ui->actionTransmit_View, SIGNAL(triggered()), this, SLOT(addRawTxWidget()));
    connect(ui->actionGenerator_View, SIGNAL(triggered()), this, SLOT(on_actionGenerator_View_triggered()));

    QAction *actionStandaloneGraph = new QAction(tr("Standalone Graph"), this);
    actionStandaloneGraph->setShortcut(QKeySequence("Ctrl+Shift+B"));
    ui->menuWindow->addAction(actionStandaloneGraph);
    connect(actionStandaloneGraph, &QAction::triggered, this, &MainWindow::createStandaloneGraphWindow);

    QAction *actionReplayView = new QAction(tr("Replay View"), this);
    ui->menuWindow->addAction(actionReplayView);
    connect(ui->actionGenerator_View, &QAction::triggered, this, [this](){ addTxGeneratorWidget(); });
    connect(actionReplayView, &QAction::triggered, this, [this](){ addReplayWidget(); });
    connect(ui->actionScript_View, &QAction::triggered, this, [this](){ addScriptWidget(); });
    connect(ui->btnStartMeasurement, SIGNAL(released()), this, SLOT(startMeasurement()));
    connect(ui->actionStop_Measurement, SIGNAL(triggered()), this, SLOT(stopMeasurement()));
    connect(ui->btnStopMeasurement, SIGNAL(released()), this, SLOT(stopMeasurement()));
    connect(ui->btnSetupMeasurement, SIGNAL(released()), this, SLOT(showSetupDialog()));

    connect(&backend(), SIGNAL(beginMeasurement()), this, SLOT(updateMeasurementActions()));
    connect(&backend(), SIGNAL(endMeasurement()), this, SLOT(updateMeasurementActions()));
    updateMeasurementActions();

    connect(ui->actionSave_Trace_to_file, SIGNAL(triggered(bool)), this, SLOT(saveTraceToFile()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));


#if defined(__linux__)
    Backend::instance().addCanDriver(*(new SocketCanDriver(Backend::instance())));
#else
    Backend::instance().addCanDriver(*(new CandleApiDriver(Backend::instance())));
#endif
    Backend::instance().addCanDriver(*(new SLCANDriver(Backend::instance())));
    Backend::instance().addCanDriver(*(new GrIPDriver(Backend::instance())));
    // Backend::instance().addCanDriver(*(new CANBlasterDriver(Backend::instance())));

    setWorkspaceModified(false);
    newWorkspace();

    // NOTE: must be called after drivers/plugins are initialized
    _setupDlg = new SetupDialog(Backend::instance(), 0);

    _showSetupDialog_first = false;

    // Open Standalone Graph Button
    QPushButton *btnOpenGraph = new QPushButton(tr("Graph"), this);
    btnOpenGraph->setIcon(QIcon(":/assets/graph.svg"));
    btnOpenGraph->setToolTip(tr("Open Standalone Graph Window (Ctrl+Shift+B)"));
    btnOpenGraph->setCursor(Qt::PointingHandCursor);
    ui->horizontalLayoutControls->insertWidget(3, btnOpenGraph); // Insert after Setup Interface button

    _actionThemeAuto = new QAction(tr("System Default"), this);
    _actionThemeLight = new QAction(tr("Light"), this);
    _actionThemeDark = new QAction(tr("Dark"), this);

    _actionThemeAuto->setCheckable(true);
    _actionThemeLight->setCheckable(true);
    _actionThemeDark->setCheckable(true);

    _themeActionGroup = new QActionGroup(this);
    _themeActionGroup->addAction(_actionThemeAuto);
    _themeActionGroup->addAction(_actionThemeLight);
    _themeActionGroup->addAction(_actionThemeDark);
    _actionThemeAuto->setChecked(true);

    QMenu *menuTheme = new QMenu(tr("&Theme"), this);
    menuTheme->addAction(_actionThemeAuto);
    menuTheme->addAction(_actionThemeLight);
    menuTheme->addAction(_actionThemeDark);

    ui->menuWindow->addSeparator();
    ui->menuWindow->addMenu(menuTheme);

    QAction *actionThemeSetColor = new QAction(tr("Set Trace Text Color..."), this);
    QAction *actionThemeResetColor = new QAction(tr("Reset Trace Text Color"), this);
    menuTheme->addSeparator();
    menuTheme->addAction(actionThemeSetColor);
    menuTheme->addAction(actionThemeResetColor);

    connect(_actionThemeAuto, &QAction::triggered, this, [this]() { this->setTheme(QStringLiteral("auto")); });
    connect(_actionThemeLight, &QAction::triggered, this, [this]() { this->setTheme(QStringLiteral("light")); });
    connect(_actionThemeDark, &QAction::triggered, this, [this]() { this->setTheme(QStringLiteral("dark")); });

    connect(actionThemeSetColor, &QAction::triggered, this, [this]() {
        QColor initColor = ThemeManager::instance().customTraceTextColor();
        if (!initColor.isValid()) initColor = ThemeManager::instance().colors().text;
        QColor target = QColorDialog::getColor(initColor, this, tr("Select Trace Text Color"));
        if (target.isValid()) {
            ThemeManager::instance().setCustomTraceTextColor(target);
            setWorkspaceModified(true);
        }
    });

    connect(actionThemeResetColor, &QAction::triggered, this, [this]() {
        ThemeManager::instance().setCustomTraceTextColor(QColor());
        setWorkspaceModified(true);
    });

    _btnTheme = new QPushButton(this);
    _btnTheme->setFixedSize(32, 32);
    _btnTheme->setCursor(Qt::PointingHandCursor);
    _btnTheme->setFlat(true);
    connect(_btnTheme, &QPushButton::clicked, this, &MainWindow::showThemeToolbarMenu);
    ui->horizontalLayoutControls->addWidget(_btnTheme);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) {
        if (_currentTheme == QLatin1String("auto")) {
            setTheme(QStringLiteral("auto"));
        }
    });
#endif

    connect(btnOpenGraph, &QPushButton::clicked, this, &MainWindow::createStandaloneGraphWindow);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateMeasurementActions()
{
    bool running = backend().isMeasurementRunning();
    ui->actionStart_Measurement->setEnabled(!running);
    ui->actionSetup->setEnabled(!running);
    ui->actionStop_Measurement->setEnabled(running);

    ui->btnStartMeasurement->setEnabled(!running);
    ui->btnSetupMeasurement->setEnabled(!running);
    ui->btnStopMeasurement->setEnabled(running);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (askSaveBecauseWorkspaceModified()!=QMessageBox::Cancel)
    {
        backend().stopMeasurement();
        
        // Auto-save to the current workspace file if one is set
        if (!_workspaceFileName.isEmpty())
        {
            saveWorkspaceToFile(_workspaceFileName);
        }
        
        event->accept();
    }
    else
    {
        event->ignore();
    }

    /*QSettings settings("MyCompany", "MyApp");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    QMainWindow::closeEvent(event);*/
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!_themeResolvedAfterShow) {
        _themeResolvedAfterShow = true;
        setTheme(_currentTheme);
    }
}

void MainWindow::showThemeToolbarMenu()
{
    QMenu menu(this);
    menu.addAction(_actionThemeAuto);
    menu.addAction(_actionThemeLight);
    menu.addAction(_actionThemeDark);
    menu.exec(_btnTheme->mapToGlobal(QPoint(0, _btnTheme->height())));
}

/*void MainWindow::readSettings()
{
    QSettings settings("MyCompany", "MyApp");
    restoreGeometry(settings.value("myWidget/geometry").toByteArray());
    restoreState(settings.value("myWidget/windowState").toByteArray());
}*/

Backend &MainWindow::backend()
{
    return Backend::instance();
}

QMainWindow *MainWindow::createTab(QString title)
{
    QMainWindow *mm = new QMainWindow(this);
    mm->setAutoFillBackground(true);
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
    while (ui->mainTabs->count() > 0) {
        QWidget *w = ui->mainTabs->widget(0);
        ui->mainTabs->removeTab(0);
        delete w;
    }

    // Close and clear standalone windows to prevent dangling pointers to signals
    while (!_standaloneGraphWindows.isEmpty()) {
        GraphWindow *gw = _standaloneGraphWindows.takeFirst();
        if (gw) {
            gw->close(); // This will trigger WA_DeleteOnClose
        }
    }

    _workspaceFileName.clear();
    setWorkspaceModified(false);
}

bool MainWindow::loadWorkspaceTab(QDomElement el)
{
    QMainWindow *mw = 0;
    QString type = el.attribute("type");    
    if (type=="TraceWindow")
    {
        mw = createTraceWindow(el.attribute("title"));
    }
    else if (type=="GraphWindow")
    {
        mw = createGraphWindow(el.attribute("title"));
    }
    else
    {
        return false;
    }

    if (mw)
    {
        ConfigurableWidget *mdi = dynamic_cast<ConfigurableWidget*>(mw->centralWidget());
        if (mdi)
        {
            mdi->loadXML(backend(), el);
        }

        ScriptWindow *script = mw->findChild<ScriptWindow *>();
        QDomElement scriptEl = el.firstChildElement("scriptwindow");
        if (script && !scriptEl.isNull())
            script->loadXML(backend(), scriptEl);
    }

    return true;
}

bool MainWindow::loadWorkspaceSetup(QDomElement el)
{
    MeasurementSetup setup(&backend());
    if (setup.loadXML(backend(), el))
    {
        backend().setSetup(setup);
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::loadWorkspaceFromFile(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        log_error(QString("Cannot open workspace settings file: %1").arg(filename));
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        file.close();
        log_error(QString("Cannot load settings from file: %1").arg(filename));
        return;
    }
    file.close();

    stopAndClearMeasurement();
    clearWorkspace();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "cangaroo-workspace")
    {
        log_error(QString("Invalid workspace file format: %1").arg(filename));
        return;
    }

    QDomElement tabsRoot = root.firstChildElement("tabs");
    QDomNodeList tabs = tabsRoot.elementsByTagName("tab");
    for (int i=0; i<tabs.length(); i++)
    {
        if (!loadWorkspaceTab(tabs.item(i).toElement()))
        {
            log_warning(QString("Could not read window %1 from file: %2").arg(QString::number(i), filename));
            continue;
        }
    }

    QDomElement setupRoot = root.firstChildElement("setup");
    if (loadWorkspaceSetup(setupRoot))
    {
        _workspaceFileName = filename;
    }
    else
    {
        log_error(QString("Unable to read measurement setup from workspace config file: %1").arg(filename));
    }

    QDomElement themeRoot = root.firstChildElement("theme-settings");
    if (!themeRoot.isNull()) {
        QString savedTheme = themeRoot.attribute("theme", "auto");
        setTheme(savedTheme);
        
        QString traceColor = themeRoot.attribute("customTraceColor", "");
        if (!traceColor.isEmpty()) {
            ThemeManager::instance().setCustomTraceTextColor(QColor(traceColor));
        } else {
            ThemeManager::instance().setCustomTraceTextColor(QColor());
        }
    }

    if (ui->mainTabs->count() > 0)
    {
        ui->mainTabs->setCurrentIndex(0);
    }
    setWorkspaceModified(false);
}

bool MainWindow::saveWorkspaceToFile(QString filename)
{
    QDomDocument doc;
    QDomElement root = doc.createElement("cangaroo-workspace");
    doc.appendChild(root);

    QDomElement tabsRoot = doc.createElement("tabs");
    root.appendChild(tabsRoot);

    for (int i=0; i < ui->mainTabs->count(); i++)
    {
        QMainWindow *w = (QMainWindow*)ui->mainTabs->widget(i);

        QDomElement tabEl = doc.createElement("tab");
        tabEl.setAttribute("title", ui->mainTabs->tabText(i));

        ConfigurableWidget *mdi = dynamic_cast<ConfigurableWidget*>(w->centralWidget());
        if (!mdi->saveXML(backend(), doc, tabEl))
        {
            log_error(QString("Cannot save window settings to file: %1").arg(filename));
            return false;
        }

        tabsRoot.appendChild(tabEl);

        ScriptWindow *script = w->findChild<ScriptWindow *>();
        if (script)
        {
            QDomElement scriptEl = doc.createElement("scriptwindow");
            script->saveXML(backend(), doc, scriptEl);
            tabEl.appendChild(scriptEl);
        }
    }

    QDomElement setupRoot = doc.createElement("setup");
    if (!backend().getSetup().saveXML(backend(), doc, setupRoot))
    {
        log_error(QString("Cannot save measurement setup to file: %1").arg(filename));
        return false;
    }
    root.appendChild(setupRoot);

    QDomElement themeRoot = doc.createElement("theme-settings");
    themeRoot.setAttribute("theme", _currentTheme);
    QColor customTrace = ThemeManager::instance().customTraceTextColor();
    if (customTrace.isValid()) {
        themeRoot.setAttribute("customTraceColor", customTrace.name(QColor::HexArgb));
    }
    root.appendChild(themeRoot);

    QFile outFile(filename);
    if(outFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream( &outFile );
        stream << doc.toString();
        outFile.close();
        _workspaceFileName = filename;
        setWorkspaceModified(false);
        log_info(QString("Saved workspace settings to file: %1").arg(filename));
        return true;
    }
    else
    {
        log_error(QString("Cannot open workspace file for writing: %1").arg(filename));
        return false;
    }
}

void MainWindow::newWorkspace()
{
    if (askSaveBecauseWorkspaceModified() != QMessageBox::Cancel)
    {
        stopAndClearMeasurement();
        clearWorkspace();
        createTraceWindow();
        backend().setDefaultSetup();
        
        // Clear the workspace filename for a fresh start
        _workspaceFileName.clear();
        setWorkspaceModified(false);
    }
}

void MainWindow::loadWorkspace()
{
    if (askSaveBecauseWorkspaceModified() != QMessageBox::Cancel)
    {
        QString filename = QFileDialog::getOpenFileName(this, tr("Open workspace configuration"), "", tr("Workspace config files (*.cangaroo)"));
        if (!filename.isNull())
        {
            loadWorkspaceFromFile(filename);
        }
    }
}

bool MainWindow::saveWorkspace()
{
    if (_workspaceFileName.isEmpty())
    {
        return saveWorkspaceAs();
    }
    else
    {
        return saveWorkspaceToFile(_workspaceFileName);
    }
}

bool MainWindow::saveWorkspaceAs()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save workspace configuration"), "", tr("Workspace config files (*.cangaroo)"));
    if (!filename.isNull())
    {
        // Ensure the filename has .cangaroo extension
        if (!filename.endsWith(".cangaroo", Qt::CaseInsensitive))
        {
            filename += ".cangaroo";
        }
        return saveWorkspaceToFile(filename);
    }
    else
    {
        return false;
    }
}

void MainWindow::setWorkspaceModified(bool modified)
{
    _workspaceModified = modified;

    QString title = _baseWindowTitle;
    if (!_workspaceFileName.isEmpty())
    {
        QFileInfo fi(_workspaceFileName);
        title += " - " + fi.fileName();
    }
    if (_workspaceModified)
    {
        title += '*';
    }
    setWindowTitle(title);
}

int MainWindow::askSaveBecauseWorkspaceModified()
{
    if (_workspaceModified)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("The current workspace has been modified."));
        msgBox.setInformativeText(tr("Do you want to save your changes?"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        /*msgBox.setButtonText(QMessageBox::Save, QString(tr("Save")));
        msgBox.setButtonText(QMessageBox::Discard, QString(tr("Discard")));
        msgBox.setButtonText(QMessageBox::Cancel, QString(tr("Cancel")));*/

        int result = msgBox.exec();
        if (result == QMessageBox::Save)
        {
            if (saveWorkspace())
            {
                return QMessageBox::Save;
            }
            else
            {
                return QMessageBox::Cancel;
            }
        }
        return result;
    }
    else
    {
        return QMessageBox::Discard;
    }
}

QMainWindow *MainWindow::createTraceWindow(QString title)
{
    if (title.isNull())
    {
        title = tr("Trace");
    }
    QMainWindow *mm = createTab(title);
    TraceWindow *trace = new TraceWindow(mm, backend());
    mm->setCentralWidget(trace);

    QDockWidget *dockLogWidget = addLogWidget(mm);
    QDockWidget *dockStatusWidget = addStatusWidget(mm);
    QDockWidget *dockRawTxWidget = addRawTxWidget(mm);
    QDockWidget *dockGeneratorWidget = addTxGeneratorWidget(mm);
    QDockWidget *dockReplayWidget = addReplayWidget(mm);
    QDockWidget *dockScriptWidget = addScriptWidget(mm);

    TxGeneratorWindow *gen = qobject_cast<TxGeneratorWindow*>(dockGeneratorWidget->widget());
    RawTxWindow *rawtx = qobject_cast<RawTxWindow*>(dockRawTxWidget->widget());
    if (gen && rawtx) {
        connect(gen, &TxGeneratorWindow::messageSelected, rawtx, &RawTxWindow::setMessage);
        connect(rawtx, &RawTxWindow::messageUpdated, gen, &TxGeneratorWindow::updateMessage);
        connect(gen, &TxGeneratorWindow::loopbackFrame, trace, &TraceWindow::addMessage);
    }



    mm->splitDockWidget(dockRawTxWidget,dockLogWidget,Qt::Horizontal);
    mm->splitDockWidget(dockGeneratorWidget,dockLogWidget,Qt::Horizontal);
    mm->tabifyDockWidget(dockGeneratorWidget, dockRawTxWidget); // Generator first, Message next
    mm->tabifyDockWidget(dockGeneratorWidget, dockReplayWidget); // Replay in that group too
    mm->splitDockWidget(dockScriptWidget, dockLogWidget, Qt::Horizontal);
    mm->tabifyDockWidget(dockGeneratorWidget, dockScriptWidget);
    mm->splitDockWidget(dockStatusWidget,dockLogWidget,Qt::Horizontal);
    mm->tabifyDockWidget(dockStatusWidget, dockLogWidget); // Status first, Log next
    
    
    // Use QTimer to resize docks and ensure correct focus/visibility after layout is complete
    QTimer::singleShot(0, mm, [mm, dockLogWidget, dockRawTxWidget, dockGeneratorWidget, dockStatusWidget]() {
        dockStatusWidget->show();
        dockStatusWidget->raise();
        dockGeneratorWidget->show();
        dockGeneratorWidget->raise();
        
        mm->resizeDocks({dockLogWidget, dockRawTxWidget, dockGeneratorWidget, dockStatusWidget}, {600, 600, 600, 600}, Qt::Vertical);
        mm->resizeDocks({dockLogWidget, dockRawTxWidget, dockGeneratorWidget, dockStatusWidget}, {1200, 1200, 1200, 1200}, Qt::Horizontal);
    });

    ui->mainTabs->setCurrentWidget(mm);
    return mm;
}

QMainWindow *MainWindow::createGraphWindow(QString title)
{
    if (title.isNull())
    {
        title = tr("Graph");
    }
    QMainWindow *mm = createTab(title);
    mm->setCentralWidget(new GraphWindow(mm, backend()));
    addLogWidget(mm);

    return mm;
}

void MainWindow::createStandaloneGraphWindow()
{
    GraphWindow *gw = new GraphWindow(nullptr, backend());
    gw->setWindowTitle(tr("Standalone Graph"));
    gw->setAttribute(Qt::WA_DeleteOnClose);
    
    _standaloneGraphWindows.append(gw);
    connect(gw, &QObject::destroyed, this, [this, gw]() {
        _standaloneGraphWindows.removeAll(gw);
    });

    gw->show();
}

void MainWindow::addGraphWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Graph"), parent);
    dock->setWidget(new GraphWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
}

QDockWidget *MainWindow::addRawTxWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Message View"), parent);
    RawTxWindow *rawTx = new RawTxWindow(dock, backend());
    dock->setWidget(rawTx);
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);

    TxGeneratorWindow *gen = parent->findChild<TxGeneratorWindow*>();
    if (gen) {
        connect(gen, &TxGeneratorWindow::messageSelected, rawTx, &RawTxWindow::setMessage);
        connect(rawTx, &RawTxWindow::messageUpdated, gen, &TxGeneratorWindow::updateMessage);
    }

    return dock;
}

QDockWidget *MainWindow::addLogWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Log"), parent);
    dock->setWidget(new LogWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
    return dock;
}

QDockWidget *MainWindow::addStatusWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("CAN Status"), parent);
    dock->setWidget(new CanStatusWindow(dock, backend()));
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
    return dock;
}

QDockWidget *MainWindow::addTxGeneratorWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Generator View"), parent);
    TxGeneratorWindow *gen = new TxGeneratorWindow(dock, backend());
    dock->setWidget(gen);
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);

    RawTxWindow *rawtx = parent->findChild<RawTxWindow*>();
    if (rawtx) {
        connect(gen, &TxGeneratorWindow::messageSelected, rawtx, &RawTxWindow::setMessage);
        connect(rawtx, &RawTxWindow::messageUpdated, gen, &TxGeneratorWindow::updateMessage);
    }

    return dock;
}

QDockWidget *MainWindow::addReplayWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Replay View"), parent);
    ReplayWindow *replay = new ReplayWindow(dock, backend());
    dock->setWidget(replay);
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
    return dock;
}

QDockWidget *MainWindow::addScriptWidget(QMainWindow *parent)
{
    if (!parent)
    {
        parent = currentTab();
    }
    QDockWidget *dock = new QDockWidget(tr("Python Script"), parent);
    ScriptWindow *script = new ScriptWindow(dock, backend());
    dock->setWidget(script);
    parent->addDockWidget(Qt::BottomDockWidgetArea, dock);
    
    connect(script, &ScriptWindow::settingsChanged, this, [this]() { setWorkspaceModified(true); });
    
    return dock;
}

void MainWindow::on_actionCan_Status_View_triggered()
{
    addStatusWidget();
}

bool MainWindow::showSetupDialog()
{
    MeasurementSetup new_setup(&backend());
    new_setup.cloneFrom(backend().getSetup());
    backend().setDefaultSetup();
    if(backend().getSetup().countNetworks() == new_setup.countNetworks())
    {
        backend().setSetup(new_setup);
    }
    else
    {
        new_setup.cloneFrom(backend().getSetup());
    }
    if (_setupDlg->showSetupDialog(new_setup))
    {
        if(!_setupDlg->isReflashNetworks())
            backend().setSetup(new_setup);

        setWorkspaceModified(true);
        _showSetupDialog_first = true;
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this,
        tr("About CANgaroo"),
       "CANgaroo\n"
       "Open Source CAN bus analyzer\n"
       "\n"
       "version " CANGAROO_VERSION_STR "\n"
       "\n"
       "(c)2026 Jayachandran Dharuman"
    );
}

void MainWindow::startMeasurement()
{
    if(!_showSetupDialog_first)
    {
        backend().clearTrace();
        backend().startMeasurement();
        _showSetupDialog_first = true;
    }
    else
    {
        backend().startMeasurement();
    }
}

void MainWindow::stopMeasurement()
{
    backend().stopMeasurement();

    foreach (TxGeneratorWindow *gen, findChildren<TxGeneratorWindow*>()) {
        gen->stopAll();
    }
}

void MainWindow::saveTraceToFile()
{
    QString filters("Vector ASC (*.asc);;Linux candump (*.candump))");
    QString defaultFilter("Vector ASC (*.asc)");

    QFileDialog fileDialog(0, "Save Trace to file", QDir::currentPath(), filters);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setOption(QFileDialog::DontConfirmOverwrite,false);
    //fileDialog.setConfirmOverwrite(true);
    fileDialog.selectNameFilter(defaultFilter);
    fileDialog.setDefaultSuffix("asc");
    if (fileDialog.exec()) {
        QString filename = fileDialog.selectedFiles()[0];
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            if (filename.endsWith(".candump", Qt::CaseInsensitive))
            {
                backend().getTrace()->saveCanDump(file);
            }
            else
            {
                backend().getTrace()->saveVectorAsc(file);
            }

            file.close();
        }
        else
        {
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

void MainWindow::on_actionReport_Issue_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/OpenAutoDiagLabs/CANgaroo/issues"));
}

void MainWindow::on_actionGenerator_View_triggered()
{
    addTxGeneratorWidget();
}

void MainWindow::setTheme(const QString &theme)
{
    _currentTheme = theme;
    const bool effectiveDark = effectiveThemePreferenceIsDark(theme);
    ThemeManager::instance().applyTheme(effectiveDark ? ThemeManager::Dark : ThemeManager::Light);

    if (_actionThemeAuto) {
        _actionThemeAuto->setChecked(theme == QLatin1String("auto"));
        _actionThemeLight->setChecked(theme == QLatin1String("light"));
        _actionThemeDark->setChecked(theme == QLatin1String("dark"));
    }

    if (_btnTheme) {
        QString tip;
        if (theme == QLatin1String("auto")) {
            tip = tr("Theme: System (%1)").arg(effectiveDark ? tr("Dark") : tr("Light"));
        } else if (theme == QLatin1String("light")) {
            tip = tr("Theme: Light");
        } else {
            tip = tr("Theme: Dark");
        }
        _btnTheme->setToolTip(tip);
        _btnTheme->setText(effectiveDark ? QStringLiteral("☀") : QStringLiteral("🌙"));
        _btnTheme->setStyleSheet(
            "QPushButton {"
            "  font-size: 18px;"
            "  border-radius: 16px;"
            "  background: transparent;"
            "  color: " + QString(effectiveDark ? "white" : "black") + ";"
            "}"
            "QPushButton:hover {"
            "  background: rgba(" + QString(effectiveDark ? "255, 255, 255" : "0, 0, 0") + ", 0.1);"
            "}"
        );
    }
}

