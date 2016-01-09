#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QObject>
#include <QWidget>

class MdiWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MdiWindow(QWidget *parent = 0);

signals:

public slots:
};

#endif // MDIWINDOW_H
