#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <Backend.h>

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
    QMdiSubWindow *createGraphView();
    void setActiveSubWindow(QWidget *window);
    bool showSetupDialog();
    void showAboutDialog();
    void startMeasurement();
    void stopMeasurement();
    void saveTraceToFile();

    void updateMeasurementActions();

private:
    Ui::MainWindow *ui;

    Backend backend;
    Logger *_logger;

    TraceWindow *activeMdiChild();
    QSignalMapper *windowMapper;

    QMdiSubWindow *createSubWindow(QWidget *window);

};

#endif // MAINWINDOW_H
