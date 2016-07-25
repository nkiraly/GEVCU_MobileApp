#include "blehandler.h"

BLEHandler::BLEHandler()
{
    discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);

    connect(discoveryAgent, SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo&)),
            this, SLOT(addDevice(const QBluetoothDeviceInfo&)));
    connect(discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
            this, SLOT(deviceScanError(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(discoveryAgent, SIGNAL(finished()), this, SLOT(scanFinished()));

    discoveryAgent->start();
}


BLEHandler::~BLEHandler()
{
    qDeleteAll(bleDevices);
    bleDevices.clear();
}

void BLEHandler::deviceSearch()
{
    discoveryAgent->stop();
    qDeleteAll(bleDevices);
    bleDevices.clear();
    discoveryAgent->start();
}

void BLEHandler::addDevice(const QBluetoothDeviceInfo &device)
{
    if (device.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration) {
        qWarning() << "Discovered LE Device name: " << device.name() << " Address: "
                   << device.address().toString();
        BLEDeviceInfo *dev = new BLEDeviceInfo();
        dev->setDevice(device);
        bleDevices.append(dev);
    }
}

void BLEHandler::scanFinished()
{
    if (bleDevices.size() == 0)
    {
        //setMessage("No Low Energy devices found");
        qWarning() << "No BLE devices found!";
    }
    //Q_EMIT nameChanged();
    for (int i = 0; i < bleDevices.size(); i++)
    {
        if ( ((BLEDeviceInfo*)bleDevices.at(i))->getName().contains("GEVCU"))
        {
            connectToService(((BLEDeviceInfo*)bleDevices.at(i))->getAddress() );
            return;
        }
    }
    qWarning() << "No GEVCU devices found to connect to!";
}

void BLEHandler::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    //if (error == QBluetoothDeviceDiscoveryAgent::PoweredOffError)
        //setMessage("The Bluetooth adaptor is powered off, power it on before doing discovery.");
    //else if (error == QBluetoothDeviceDiscoveryAgent::InputOutputError)
        //setMessage("Writing or reading from the device resulted in an error.");
    //else
        //setMessage("An unknown error has occurred.");
}


//QVariant BLEHandler::name()
//{
//    return QVariant::fromValue(bleDevices);
//}

void BLEHandler::connectToService(const QString &address)
{
    bool deviceFound = false;
    for (int i = 0; i < bleDevices.size(); i++) {
        if (((BLEDeviceInfo*)bleDevices.at(i))->getAddress() == address ) {
            currentDevice.setDevice(((BLEDeviceInfo*)bleDevices.at(i))->getDevice());
            //setMessage("Connecting to device...");
            qWarning() << "Connecting to device " << ((BLEDeviceInfo*)bleDevices.at(i))->getAddress();
            deviceFound = true;
            break;
        }
    }
    // we are running demo mode
    if (!deviceFound) {
        //startDemo();
        return;
    }

    if (bleController) {
        bleController->disconnectFromDevice();
        delete bleController;
        bleController = 0;
    }

    bleController = new QLowEnergyController(currentDevice.getDevice(), this);
    connect(bleController, SIGNAL(serviceDiscovered(QBluetoothUuid)),
            this, SLOT(serviceDiscovered(QBluetoothUuid)));
    connect(bleController, SIGNAL(discoveryFinished()),
            this, SLOT(serviceScanDone()));
    connect(bleController, SIGNAL(error(QLowEnergyController::Error)),
            this, SLOT(controllerError(QLowEnergyController::Error)));
    connect(bleController, SIGNAL(connected()),
            this, SLOT(deviceConnected()));
    connect(bleController, SIGNAL(disconnected()),
            this, SLOT(deviceDisconnected()));

    bleController->connectToDevice();
}

void BLEHandler::deviceConnected()
{
    bleController->discoverServices();
}

void BLEHandler::deviceDisconnected()
{
    //setMessage("Heart Rate service disconnected");
    qWarning() << "Remote device disconnected";
}

void BLEHandler::serviceDiscovered(const QBluetoothUuid &gatt)
{
    if (gatt == QBluetoothUuid(QBluetoothUuid::HeartRate)) {
        //setMessage("GEVCU service discovered. Waiting for service scan to be done...");
        foundGEVCUService = true;
    }
}

void BLEHandler::serviceScanDone()
{
    delete bleService;
    bleService = 0;

    if (foundGEVCUService) {
        //setMessage("Connecting to service...");
        bleService = bleController->createServiceObject(QBluetoothUuid(QBluetoothUuid::HeartRate), this);
    }

    if (!bleService) {
        //setMessage("GEVCU Service not found.");
        return;
    }

    connect(bleService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)),
            this, SLOT(serviceStateChanged(QLowEnergyService::ServiceState)));
    connect(bleService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
            this, SLOT(updateHeartRateValue(QLowEnergyCharacteristic,QByteArray)));
    connect(bleService, SIGNAL(descriptorWritten(QLowEnergyDescriptor,QByteArray)),
            this, SLOT(confirmedDescriptorWrite(QLowEnergyDescriptor,QByteArray)));

    bleService->discoverDetails();
}

void BLEHandler::disconnectService()
{
    foundGEVCUService = false;

    if (bleDevices.isEmpty()) {
        //if (timer)
            //timer->stop();
        return;
    }

    //disable notifications
    if (bleDescriptor.isValid() && bleService) {
        bleService->writeDescriptor(bleDescriptor, QByteArray::fromHex("0000"));
    } else {
        bleController->disconnectFromDevice();
        delete bleService;
        bleService = 0;
    }
}

void BLEHandler::controllerError(QLowEnergyController::Error error)
{
    //setMessage("Cannot connect to remote device.");
    qWarning() << "Controller Error:" << error;
}


void BLEHandler::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::ServiceDiscovered:
    {
        /*
        const QLowEnergyCharacteristic hrChar = m_service->characteristic(
                    QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement));
        if (!hrChar.isValid()) {
            setMessage("HR Data not found.");
            break;
        }

        const QLowEnergyDescriptor m_notificationDesc = hrChar.descriptor(
                    QBluetoothUuid::ClientCharacteristicConfiguration);
        if (m_notificationDesc.isValid()) {
            m_service->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            setMessage("Measuring");
            m_start = QDateTime::currentDateTime();
        }
*/
        break;
    }
    default:
        //nothing for now
        break;
    }
}


void BLEHandler::serviceError(QLowEnergyService::ServiceError e)
{
    switch (e) {
    case QLowEnergyService::DescriptorWriteError:
        //setMessage("Cannot obtain HR notifications");
        break;
    default:
        qWarning() << "HR service error:" << e;
    }
}

//reading a value
/*
void HeartRate::updateHeartRateValue(const QLowEnergyCharacteristic &c,
                                     const QByteArray &value)
{
    // ignore any other characteristic change -> shouldn't really happen though
    if (c.uuid() != QBluetoothUuid(QBluetoothUuid::HeartRateMeasurement))
        return;


    const quint8 *data = reinterpret_cast<const quint8 *>(value.constData());
    quint8 flags = data[0];

    //Heart Rate
    if (flags & 0x1) { // HR 16 bit? otherwise 8 bit
        const quint16 heartRate = qFromLittleEndian<quint16>(data[1]);
        //qDebug() << "16 bit HR value:" << heartRate;
        m_measurements.append(heartRate);
    } else {
        const quint8 *heartRate = &data[1];
        m_measurements.append(*heartRate);
        //qDebug() << "8 bit HR value:" << *heartRate;
    }

    //Energy Expended
    if (flags & 0x8) {
        int index = (flags & 0x1) ? 5 : 3;
        const quint16 energy = qFromLittleEndian<quint16>(data[index]);
        qDebug() << "Used Energy:" << energy;
    }

    Q_EMIT hrChanged();
}
*/

void BLEHandler::confirmedDescriptorWrite(const QLowEnergyDescriptor &d,
                                         const QByteArray &value)
{
    if (d.isValid() && d == bleDescriptor && value == QByteArray("0000")) {
        //disabled notifications -> assume disconnect intent
        bleController->disconnectFromDevice();
        delete bleService;
        bleService = 0;
    }
}

QString BLEHandler::deviceAddress() const
{
    return currentDevice.getAddress();
}

int BLEHandler::numDevices() const
{
    return bleDevices.size();
}




