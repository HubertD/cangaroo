#ifndef MEASUREMENTSETUP_H
#define MEASUREMENTSETUP_H

#include <QObject>
#include <QList>

class MeasurementNetwork;
class CanTrace;
class CanMessage;
class CanInterface;
class CanDbMessage;

class MeasurementSetup : public QObject
{
    Q_OBJECT

public:
    explicit MeasurementSetup(QObject *parent);
    virtual ~MeasurementSetup();
    MeasurementNetwork *createNetwork();

    CanDbMessage *findDbMessage(const CanMessage &msg);
    QString getInterfaceName(const CanInterface &interface);

    int countNetworks() const;
    MeasurementNetwork *getNetwork(int index) const;
    QList<MeasurementNetwork*> getNetworks();

    void cloneFrom(MeasurementSetup &origin);

private:
    QList<MeasurementNetwork*> _networks;
};

#endif // MEASUREMENTSETUP_H
