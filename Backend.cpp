#include "Backend.h"

#include <QDebug>

#include <model/CanTrace.h>
#include <model/MeasurementSetup.h>
#include <model/MeasurementNetwork.h>
#include <drivers/CanListener.h>
#include <drivers/CanInterface.h>
#include <drivers/CanInterfaceProvider.h>
#include <drivers/socketcan/SocketCanInterfaceProvider.h>

Backend::Backend(QObject *parent)
  : QObject(parent),
    _measurementRunning(false)
{
    qRegisterMetaType<CanMessage>("CanMessage");

    _socketcan = new SocketCanInterfaceProvider();
    _setup = createDefaultSetup();
    _trace = new CanTrace(this, _setup, 100);
}

Backend::~Backend()
{
    delete _trace;
    delete _setup;
    delete _socketcan;
}

bool Backend::startMeasurement()
{
    qDebug("starting measurement");
    int i=0;
    foreach (MeasurementNetwork *network, _setup->getNetworks()) {
        i++;
        foreach (pCanInterface intf, network->_canInterfaces) {
            intf->setId(i);
            intf->open();

            qDebug() << "listening on interface" << intf->getName();

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
    _socketcan->update();

    MeasurementSetup *defaultSetup = new MeasurementSetup(this);
    int i = 1;
    foreach (pCanInterface intf, _socketcan->getInterfaceList()) {
        MeasurementNetwork *network = defaultSetup->createNetwork();
        network->setName(QString().sprintf("Network %d", i++));
        network->addCanInterface(intf);
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
    _trace->setSetup(_setup);
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

