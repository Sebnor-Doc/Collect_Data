#include "LocalizationThread.h"
#include <time.h>
#include <QUrl>
#include <QDebug>

LocalizationThread::LocalizationThread(QObject *parent, ReadSensors *rs, QString readSensorDataFileName)
    : QThread(parent)
{
    stop = true;
    this->rs = rs;
    sensorData_counter = 0;
}


void LocalizationThread::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(HighestPriority);
    }
}

void LocalizationThread::run()
{
    if (IMMEDIATE_OR_REALTIME)
    {
        readRawFile();
    }

    while (!stop)
    {
        mutex.lock();
        computeLocalization();
        if (save)
        {
            // Add on magnet position
            (*localizationOutputFile_stream) << magnet.m_position(0) << " "
                                             << magnet.m_position(1) << " "
                                             << magnet.m_position(2) << " "
                                             << magnet.m_theta       << " "
                                             << magnet.m_phi         << " " ;


            (*localizationOutputFile_stream)<<QDateTime::currentDateTime().toMSecsSinceEpoch()<<endl;

        }
        mutex.unlock();
    }
}

LocalizationThread::~LocalizationThread()
{
    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}

void LocalizationThread::Stop()
{
    stop = true;
    if (save)
        stopSavingToFile();
}

void LocalizationThread::msleep(int ms){
    QThread::msleep(ms);
}

bool LocalizationThread::isStopped() const{
    return this->stop;
}

void LocalizationThread::updateSensorFields()
{
    if (!IMMEDIATE_OR_REALTIME)
    {
        for (int i=0; i<NUM_OF_SENSORS;i++)
        {
            sensors[i]->currentField(rs->getSensorData(i/4,i%4,0),
                                     rs->getSensorData(i/4,i%4,1),
                                     rs->getSensorData(i/4,i%4,2));
            sensors[i]->calibrate();
        }
    }
    else
    {
        for (int i=0; i<NUM_OF_SENSORS;i++)
        {
            sensors[i]->currentField(sensorData.at(sensorData_counter)(i*3),
                                     sensorData.at(sensorData_counter)(i*3+1),
                                     sensorData.at(sensorData_counter)(i*3+2));
            sensors[i]->calibrate();
        }
        sensorData_counter++;
        if (sensorData_counter == sensorData_length)
            stop = true;
    }
}

Magnet LocalizationThread::localizer(Magnet &magnet)
{
    const int n = 5;
    double start[n] = {magnet.position()(0),magnet.position()(1),magnet.position()(2),magnet.theta(),magnet.phi()};
    double step[n] = {0.0001,0.0001,0.0001,PI/720.,PI/720.};
    double reqmin = 1.0E-10;
    double xmin[n];
    int konvge = 250;
    int kcount = 500;
    int icount;
    int ifault;
    int numres;
    double ynewlo = localizationErrorFunction(start);
    nelmin ( localizationErrorFunction, n, start, xmin, &ynewlo, reqmin, step, konvge, kcount, &icount, &numres, &ifault );
    Magnet newMagnet(magnet);
    newMagnet.position(CImg<double>().assign(1,3).fill(xmin[0],xmin[1],xmin[2])).moment(xmin[3],xmin[4]);
    return newMagnet;
}

void LocalizationThread::computeLocalization()
{
    updateSensorFields();
    magnet = localizer(magnet);
}

CImg<double> LocalizationThread::currentPosition()
{
    return magnet.position();
}

void LocalizationThread::setFileLocationAndName(QString filename)
{
    this->filename = filename;
}

void LocalizationThread::saveToFile()
{
    mutex.lock();
    save = true;
    localizationOutputFile.setFileName(filename);
    localizationOutputFile_stream = new QTextStream(&localizationOutputFile);
    localizationOutputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    mutex.unlock();
}

void LocalizationThread::stopSavingToFile()
{
    if (save)
    {
        mutex.lock();
        save = false;
        localizationOutputFile.close();
        mutex.unlock();
    }
}

void LocalizationThread::setSensorFile(QString readSensorDataFileName)
{
    this->readSensorDataFileName = readSensorDataFileName;
}

void LocalizationThread::readRawFile()
{
    QFile input(readSensorDataFileName);
    QTextStream in(&input);
    QStringList reading;

    if (input.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!in.atEnd())
        {
            QString t = in.readLine();
            // Make sure its space delimited
            reading = t.split(" ");
            CImg<double> temp;
            temp.assign(1,NUM_OF_SENSORS*3);
            for (int i=0; i<reading.length()-1; i++)
            {
                temp(i) = reading[i].toDouble();
            }
            sensorData.push_back(temp);
        }
        input.close();
    }
    sensorData_length = sensorData.size();
}
