#include "ReadSensors.h"
#include <time.h>
#include <QUrl>
#include <QDebug>
#include <QDateTime>

#include <QSerialPortInfo>


ReadSensors::ReadSensors(QObject *parent)
    : QThread(parent)
{
    stop = true;
    save = false;

    // Connect to Mojo by identifying its COM Port
    QSerialPortInfo mojoComPort;
    QList<QSerialPortInfo> listOfPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, listOfPorts) {
        if (port.productIdentifier() == 32769) {
            mojoComPort = port;
            break;
        }
    }

    // Set Mojo Serial Port
    if (!mojoComPort.portName().isEmpty()) {
        qDebug() << "Mojo Com Port found at: " << mojoComPort.portName();
        setCOMPort(mojoComPort.portName());
    }
    else {
        qDebug() << "ERROR: NO COM PORT ASSOCIATED TO MOJO IS FOUND!!";
    }
}


void ReadSensors::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(HighestPriority);
    }
}

void ReadSensors::run()
{
    if (!checkCOMPorts()) {
        stop = true;
        qDebug() << "ERROR: CHECK COM PORT FAILED";
        throw std::runtime_error("");
    }

    qRegisterMetaType<MagData*>("MagData*");
    connect(sp, SIGNAL(newPacket(MagData*)), this, SLOT(processPacket(MagData*)));

    while (!stop) {
        mutex.lock();
        sp->getSinglePacket();
        mutex.unlock();
    }

    disconnect(sp, SIGNAL(newPacket(MagData*)), this, SLOT(processPacket(MagData*)));
}


void ReadSensors::processPacket(MagData *packet) {
    mutex.lock();
    // Save raw magnetic information
    if (save)
    {
        for (int i = 0; i < packet->size(); i++) {
            (*sensorOutputFile_stream) << packet->at(i) << " ";
        }

        (*sensorOutputFile_stream) << QDateTime::currentDateTime().toMSecsSinceEpoch() << endl;
    }

    emit newPacketAvail(packet);
    mutex.unlock();
}


short ReadSensors::getSensorData(int pcb, int sensor, int dim)
{
    return sp->getSensorData(pcb, sensor, dim);
}

/* Manage thread status */
void ReadSensors::Stop()
{
    stop = true;
}

bool ReadSensors::isStopped() const{
    return this->stop;
}

void ReadSensors::beginRecording()
{
    stop = false;

}

/* Manage Saving */
void ReadSensors::setFileLocation(QString filename)
{
    this->filename = filename;
}

void ReadSensors::saveToFile()
{
    mutex.lock();
    save = true;
    sensorOutputFile.setFileName(filename);
    sensorOutputFile_stream = new QTextStream(&sensorOutputFile);
    sensorOutputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    mutex.unlock();
}

void ReadSensors::stopSavingToFile()
{
    mutex.lock();
    save = false;
    sensorOutputFile.close();
    mutex.unlock();
}


/* Manage connection to Mojo */
void ReadSensors::setCOMPort(QString source)
{
    this->source = source;
    try
    {
        sp = new MOJOSerialPort(source.toStdString());
        qDebug() << "Mojo connected successfully!";
        sp->getSinglePacket();
    }
    catch(...)
    {
        qDebug()<<"ERROR: MOJO PORT FOUND BUT CANNOT CONNECT: " << source;
    }
}

bool ReadSensors::checkCOMPorts()
{
    mutex.lock();
    sp->getSinglePacket();
//    if (sp->rawData.size() == EXPECTEDBYTES-NOOFBYTESINPC)
    if (sp->rawData.size() == (3*NUM_OF_SENSORS))
    {
        mutex.unlock();
        return true;
    }
    else
    {
        qDebug()<<"ERROR: PACKET SIZE RECEIVED FROM MOJO IS INCORRECT" << endl;
        mutex.unlock();
        return false;
    }
}

/* Other */
ReadSensors::~ReadSensors()
{
    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}
