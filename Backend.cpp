#include "Backend.h"

#include <QDebug>

#include <model/CanTrace.h>
#include <model/MeasurementSetup.h>
#include <model/MeasurementNetwork.h>
#include <model/MeasurementInterface.h>
#include <driver/CanDriver.h>
#include <driver/CanInterface.h>
#include <driver/CanListener.h>

Backend::Backend(QObject *parent)
  : QObject(parent),
    _measurementRunning(false)
{
    qRegisterMetaType<CanMessage>("CanMessage");

    _setup = createDefaultSetup();
    _trace = new CanTrace(this, 100);
}

Backend::~Backend()
{
    delete _trace;
    delete _setup;
}

void Backend::addCanDriver(CanDriver *driver)
{
    _drivers.append(driver);
}

bool Backend::startMeasurement()
{
    qDebug("starting measurement");
    int i=0;
    foreach (MeasurementNetwork *network, _setup->getNetworks()) {
        i++;
        foreach (MeasurementInterface *mi, network->interfaces()) {

            pCanInterface intf = mi->canInterface();
            intf->setId(i);
            intf->open();

            qDebug() << "listening on interface" << intf->getName();

            if (intf->getBitrate() != mi->bitrate()) {
                qInfo() << "setting bitrate on" << intf->getName() << " from " << intf->getBitrate() << " to " << mi->bitrate();
                intf->setBitrate(mi->bitrate());
            }

            CanListener *listener = new CanListener(0, intf);
            connect(listener, SIGNAL(messageReceived(CanMessage)), _trace, SLOT(enqueueMessage(CanMessage)));
            listener->startThread();
            _listeners.append(listener);
        }
    }

    _measurementRunning = true;
    emit beginMeasurement();
    return true;
}

bool Backend::stopMeasurement()
{
    foreach (CanListener *listener, _listeners) {
        listener->requestStop();
    }

    foreach (CanListener *listener, _listeners) {
        listener->waitFinish();
        qDebug() << "closing interface" << listener->getInterface()->getName();
        listener->getInterface()->close();
    }

    qDeleteAll(_listeners);
    _listeners.clear();

    qDebug("measurement stopped");

    _measurementRunning = false;

    emit endMeasurement();
    return true;
}

bool Backend::isMeasurementRunning()
{
    return _measurementRunning;
}

MeasurementSetup *Backend::createDefaultSetup()
{

    MeasurementSetup *defaultSetup = new MeasurementSetup(this);
    int i = 1;

    foreach (CanDriver *driver, _drivers) {
        driver->update();
        foreach (pCanInterface intf, driver->getInterfaceList()) {
            MeasurementNetwork *network = defaultSetup->createNetwork();
            network->setName(QString().sprintf("Network %d", i++));

            MeasurementInterface *mi = new MeasurementInterface();
            mi->setCanInterface(intf);
            mi->setBitrate(500000);
            network->addInterface(mi);
        }
    }

    return defaultSetup;
}

MeasurementSetup *Backend::getSetup()
{
    return _setup;
}

void Backend::setSetup(MeasurementSetup *setup)
{
    MeasurementSetup *old_setup = _setup;
    _setup = setup;
    delete old_setup;
}

void Backend::saveCanDump(QString filename)
{
    _trace->saveCanDump(filename);
}

CanTrace *Backend::getTrace()
{
    return _trace;
}

void Backend::clearTrace()
{
    _trace->clear();
}

CanDbMessage *Backend::findDbMessage(const CanMessage &msg)
{
    return _setup->findDbMessage(msg);
}

CanInterfaceList Backend::getInterfaceList()
{
    CanInterfaceList result;
    foreach (CanDriver *driver, _drivers) {
        foreach (pCanInterface intf, driver->getInterfaceList()) {
            result.append(intf);
        }
    }
    return result;
}

QString Backend::getInterfaceName(const CanInterface &interface)
{
    return _setup->getInterfaceName(interface);
}
