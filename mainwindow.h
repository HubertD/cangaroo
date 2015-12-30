#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TraceView;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

class MeasurementSetup;
class SocketCanInterfaceProvider;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

public slots:
    TraceView *createMdiChild();
    void setActiveSubWindow(QWidget *window);

private:
    Ui::MainWindow *ui;

    MeasurementSetup *setup;
    SocketCanInterfaceProvider *_provider;

    TraceView *activeMdiChild();
    QSignalMapper *windowMapper;

    void startup();

};

#endif // MAINWINDOW_H
