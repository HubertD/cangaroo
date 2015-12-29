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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    TraceView *createMdiChild();
    void setActiveSubWindow(QWidget *window);
    void updateMenus();

private:
    MeasurementSetup *setup;
    SocketCanInterfaceProvider *_provider;

    TraceView *activeMdiChild();
    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;

    void startup();

};

#endif // MAINWINDOW_H
