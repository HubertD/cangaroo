#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <Backend.h>
#include <driver/socketcan/SocketCanDriver.h>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QWidget;
class QSignalMapper;
class QDomElement;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

class MdiWindow;
class Logger;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Logger *logger, QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    QMdiSubWindow *createTraceWindow();
    QMdiSubWindow *createLogWindow();
    QMdiSubWindow *createGraphWindow();
    void setActiveSubWindow(QWidget *window);
    bool showSetupDialog();
    void showAboutDialog();

    void startMeasurement();
    void stopMeasurement();
    void saveTraceToFile();

    void updateMeasurementActions();

private slots:
    void on_workspace_modified();
    void on_action_WorkspaceNew_triggered();
    void on_action_WorkspaceOpen_triggered();
    void on_action_WorkspaceSave_triggered();
    void on_action_WorkspaceSaveAs_triggered();
    void on_action_TraceClear_triggered();

private:
    Ui::MainWindow *ui;

    Backend backend;
    Logger *_logger;
    SocketCanDriver _socketcan;

    bool _workspaceModified;
    QString _workspaceFileName;

    QString _baseWindowTitle;

    QSignalMapper *windowMapper;

    QMdiSubWindow *createSubWindow(MdiWindow *window);

    void stopAndClearMeasurement();

    bool loadWorkspaceWindow(QDomElement el);
    bool loadWorkspaceSetup(QDomElement el);
    void loadWorkspaceFromFile(QString filename);
    bool saveWorkspaceToFile(QString filename);

    void newWorkspace();
    void loadWorkspace();
    bool saveWorkspace();
    bool saveWorkspaceAs();

    void setWorkspaceModified(bool modified);
    int askSaveBecauseWorkspaceModified();

};

#endif // MAINWINDOW_H
