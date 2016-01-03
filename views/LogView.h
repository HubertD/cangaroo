#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>


namespace Ui {
class LogView;
}

class Logger;

class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget *parent, Logger *logger);
    ~LogView();

public slots:
    void onLogMessage(QtMsgType type, const QString &msg);

private:
    Ui::LogView *ui;
};

#endif // LOGVIEW_H
