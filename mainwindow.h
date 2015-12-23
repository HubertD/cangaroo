#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <vector>
#include <model/CanTrace.h>
#include "views/CanMessageTraceViewModel.h"

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
    Ui::MainWindow *ui;
    CanTrace *trace;
    CanMessageTraceViewModel *model;
};

#endif // MAINWINDOW_H
