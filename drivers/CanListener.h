#ifndef CANLISTENER_H
#define CANLISTENER_H

#include <QObject>

class CanInterface;
class CanMessage;

class CanListener : public QObject
{
    Q_OBJECT
public:
    explicit CanListener(QObject *parent, CanInterface *intf);

signals:
    void messageReceived(int interface_id, const CanMessage &msg);

public slots:
    void run();

private:
    CanInterface *_intf;

};

#endif // CANLISTENER_H
