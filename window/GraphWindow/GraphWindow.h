#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <MdiWindow.h>
#include <model/MeasurementSetup.h>
#include <Backend.h>

namespace Ui {
class GraphWindow;
}

class GraphWindow : public MdiWindow
{
    Q_OBJECT

public:
    explicit GraphWindow(QWidget *parent, Backend &backend);
    ~GraphWindow();

private:
    Ui::GraphWindow *ui;
    Backend *_backend;
};

#endif // GRAPHWINDOW_H
