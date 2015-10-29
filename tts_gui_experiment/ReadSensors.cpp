#include "ReadSensors.h"
#include "common.h"
#include <time.h>
#include <QUrl>
#include <QDebug>
#include <QDateTime>


ReadSensors::ReadSensors(QObject *parent)
    : QThread(parent)
{
    stop = true;
    save = false;
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
        throw std::runtime_error("Verify if system is Low/High Density");
    }

    while (!stop)
    {
        mutex.lock();

        sp->getSinglePacket();

        //Save raw magnetic information
        if (save)
        {
            for (int i=0; i < NUM_OF_SENSORS; i++)
            {
                (*sensorOutputFile_stream) <<sp->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 0)<<" "
                                           <<sp->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 1)<<" "
                                           <<sp->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 2)<<" ";
            }
            (*sensorOutputFile_stream)<<QDateTime::currentDateTime().toMSecsSinceEpoch()<<endl;

        }
        mutex.unlock();

    }
}

bool ReadSensors::checkCOMPorts()
{
    mutex.lock();
    sp->getSinglePacket();
    if (sp->rawData.size() == EXPECTEDBYTES-NOOFBYTESINPC)
    {
        qDebug()<<"CONNECTED TO FPGA SUCCESSFULLY!"<<endl;
        mutex.unlock();
        return true;
    }
    else
    {
        qDebug()<<"COULD NOT CONNECT TO FPGA ON "<<source<<"!"<<endl;
        mutex.unlock();
        return false;
    }
}

void ReadSensors::setCOMPort(QString source)
{
    this->source = source;
    try
    {
        sp = new MOJOSerialPort(source.toStdString());
        qDebug() << "Mojo connected successfully!" << endl;
        sp->getSinglePacket();
    }
    catch(...)
    {
        qDebug()<<"Mojo port " << source << " cannot be found.\nPlease check connection to mojo." << endl;
    }
}

void ReadSensors::Stop()
{
    stop = true;
}

void ReadSensors::msleep(int ms){
    QThread::msleep(ms);
}

bool ReadSensors::isStopped() const{
    return this->stop;
}

void ReadSensors::beginRecording()
{
    stop = false;

}

void ReadSensors::endRecording()
{
    stop = true;
}

short ReadSensors::getSensorData(int pcb, int sensor, int dim)
{
    return sp->getSensorData(pcb, sensor, dim);
}

void ReadSensors::setFileLocationAndName(QString filename)
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

ReadSensors::~ReadSensors()
{
    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}
