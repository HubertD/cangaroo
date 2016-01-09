#ifndef MEASUREMENTSETUP_H
#define MEASUREMENTSETUP_H

#include <QObject>
#include <QList>
#include <QDomDocument>

class Backend;
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

    CanDbMessage *findDbMessage(const CanMessage &msg);
    QString getInterfaceName(const CanInterface &interface);

    int countNetworks() const;
    MeasurementNetwork *getNetwork(int index) const;
    MeasurementNetwork *getNetworkByName(QString name) const;
    QList<MeasurementNetwork*> getNetworks();
    MeasurementNetwork *createNetwork();
    void removeNetwork(MeasurementNetwork *network);

    void cloneFrom(MeasurementSetup &origin);
    bool saveXML(Backend &backend, QDomDocument &xml, QDomElement &root);

private:
    QList<MeasurementNetwork*> _networks;
};

#endif // MEASUREMENTSETUP_H
