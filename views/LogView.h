#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <setup/MeasurementSetup.h>

namespace Ui {
class LogView;
}

class MeasurementSetup;

class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget *parent, MeasurementSetup *setup);
    ~LogView();

private:
    Ui::LogView *ui;
    MeasurementSetup *_setup;
};

#endif // LOGVIEW_H
