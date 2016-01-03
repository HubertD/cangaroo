#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class Logger;
class TraceWindow;
class LogView;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QWidget;
class QSignalMapper;
QT_END_NAMESPACE

class CanTrace;
class MeasurementSetup;
class SocketCanInterfaceProvider;

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
    QMdiSubWindow *createTraceView();
    QMdiSubWindow *createLogView();
    QMdiSubWindow *createGraphView();
    void setActiveSubWindow(QWidget *window);
    bool showSetupDialog();
    void startMeasurement();
    void stopMeasurement();

private:
    Ui::MainWindow *ui;

    Logger *_logger;
    MeasurementSetup *_setup;
    CanTrace *_trace;

    TraceWindow *activeMdiChild();
    QSignalMapper *windowMapper;

    QMdiSubWindow *createSubWindow(QWidget *window);
    MeasurementSetup *createDefaultSetup();

};

#endif // MAINWINDOW_H
