#ifndef MEASUREMENTSETUP_H
#define MEASUREMENTSETUP_H

#include <QObject>
#include <QList>

typedef enum {
    log_level_info,
    log_level_warn,
    log_level_error,
    log_level_critical
} log_level_t;

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

    void startMeasurement();
    void stopMeasurement();

    CanTrace *getTrace();
    CanDbMessage *findDbMessage(const CanMessage *msg);
    QString getInterfaceName(CanInterface *interface);

    void log(log_level_t level, QString s);

    int countNetworks() const;
    MeasurementNetwork *getNetwork(int index) const;
    QList<MeasurementNetwork*> getNetworks();

signals:
    void appendLog(log_level_t level, QString s);

private:
    CanTrace *_trace;
    QList<MeasurementNetwork*> _networks;
};

#endif // MEASUREMENTSETUP_H
