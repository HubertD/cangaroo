#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeView>
#include <vector>
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
    void appendMessages();

private:
    Ui::MainWindow *ui;
    CanMessageTraceViewModel *model;
};

#endif // MAINWINDOW_H
