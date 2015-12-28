#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <drivers/socketcan/SocketCanInterfaceProvider.h>
#include <model/CanDb.h>
#include <model/CanTrace.h>
#include <views/LinearTraceViewModel.h>
#include <views/AggregatedTraceViewModel.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void onCbTraceTypeChanged(int i);

private:
    Ui::MainWindow *ui;
    CanDb _candb;
    CanTrace *_trace;
    SocketCanInterfaceProvider *_provider;
    LinearTraceViewModel *_linearTraceViewModel;
    AggregatedTraceViewModel *_aggregatedTraceViewModel;

};

#endif // MAINWINDOW_H
