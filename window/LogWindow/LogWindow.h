#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <MdiWindow.h>

namespace Ui {
class LogWindow;
}

class QDomDocument;
class QDomElement;
class Backend;
class Logger;

class LogWindow : public MdiWindow
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent, Logger &logger);
    ~LogWindow();

    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

public slots:
    void onLogMessage(QtMsgType type, const QString &msg);

private:
    Ui::LogWindow *ui;
};

#endif // LOGWINDOW_H
