#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <model/MeasurementSetup.h>

namespace Ui {
class GraphView;
}

class GraphView : public QWidget
{
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent, MeasurementSetup *setup);
    ~GraphView();

private:
    Ui::GraphView *ui;
    MeasurementSetup *_setup;
};

#endif // GRAPHVIEW_H
