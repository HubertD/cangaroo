#ifndef CANLISTENER_H
#define CANLISTENER_H

#include <QObject>
#include <drivers/CanInterface.h>

class CanMessage;

class CanListener : public QObject
{
    Q_OBJECT

public:
    explicit CanListener(QObject *parent, pCanInterface intf);

signals:
    void messageReceived(const CanMessage &msg);

public slots:
    void run();

private:
    pCanInterface _intf;

};

#endif // CANLISTENER_H
