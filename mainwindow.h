#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <vector>
#include <model/CanTrace.h>
#include "views/LinearTraceViewModel.h"
#include "model/CanDb.h"

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

private:
    CanDb _candb;
    Ui::MainWindow *ui;
    CanTrace *trace;
    LinearTraceViewModel *model;
};

#endif // MAINWINDOW_H
