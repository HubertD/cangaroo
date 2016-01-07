#ifndef CANLISTENER_H
#define CANLISTENER_H

#include <QObject>
#include <driver/CanInterface.h>

class QThread;
class CanMessage;

class CanListener : public QObject
{
    Q_OBJECT

public:
    explicit CanListener(QObject *parent, pCanInterface intf);
    virtual ~CanListener();

    pCanInterface getInterface();

signals:
    void messageReceived(const CanMessage &msg);

public slots:
    void run();

    void startThread();
    void requestStop();
    void waitFinish();

private:
    pCanInterface _intf;
    bool _shouldBeRunning;
    QThread *_thread;

};

#endif // CANLISTENER_H
