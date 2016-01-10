#ifndef MDIWINDOW_H
#define MDIWINDOW_H

#include <QObject>
#include <QWidget>

class Backend;
class QDomDocument;
class QDomElement;

class MdiWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MdiWindow(QWidget *parent = 0);
    virtual bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);
    virtual bool loadXML(Backend &backend, QDomElement &el);

signals:
    void settingsChanged(MdiWindow *sender);

public slots:
};

#endif // MDIWINDOW_H
