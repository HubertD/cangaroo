#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <Backend.h>
#include <driver/socketcan/SocketCanDriver.h>

class Logger;
class TraceWindow;
class LogWindow;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QWidget;
class QSignalMapper;
QT_END_NAMESPACE

namespace Ui {
class MainWindow;
}

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
    void on_action_TraceClear_triggered();

    void on_action_WorkspaceOpen_triggered();

    void on_actionSave_as_triggered();

private:
    Ui::MainWindow *ui;

    Backend backend;
    Logger *_logger;
    SocketCanDriver _socketcan;
    QString _workspaceFileName;

    QSignalMapper *windowMapper;

    QMdiSubWindow *createSubWindow(QWidget *window);

    void loadWorkspace(QString filename);
    void saveWorkspace(QString filename);

};

#endif // MAINWINDOW_H
