#ifndef LOCALIZATIONTHREAD_H
#define LOCALIZATIONTHREAD_H

#include "ReadSensors.h"
#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <QTimer>
#include "common.h"
#include "Sensor.h"
#include "Magnet.h"
#include "CImg.h"
#include <QVector>


using namespace cimg_library;

//Global Variables

Sensor *sensors[NUM_OF_SENSORS];
Magnet magnet;


double localizationErrorFunction(double x[5]) {

    Magnet testMagnet(magnet);
    testMagnet.position(CImg<double>().assign(1,3).fill(x[0],x[1],x[2]));
    testMagnet.moment(x[3],x[4]);

    double error = 0;
    for(int i=0; i<NUM_OF_SENSORS; i++)
    {
        error +=  (testMagnet.fields(sensors[i]->position())-sensors[i]->m_extrapField).magnitude();
    }

    return error;
}


class LocalizationThread: public QThread
{    Q_OBJECT
 private:
    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    ReadSensors *rs;
    Magnet magnet;
    QString filename;
    bool save;

    QFile localizationOutputFile;
    QTextStream *localizationOutputFile_stream;

    QString readSensorDataFileName;

    QVector<CImg<double>> sensorData;
    int sensorData_length;
    int sensorData_counter;

 protected:
     void run();
     void msleep(int ms);

 public:
    //Constructor
    LocalizationThread(QObject *parent = 0, ReadSensors *rs = 0, QString readSensorDataFileName = "");
    //Destructor
    ~LocalizationThread();
    //Run the webcam
    void Play();
    //Stop
    void Stop();
    //check if the LocalizationThread has been stopped
    bool isStopped() const;
    //update Sensor values
    void updateSensorFields();
    //error function
    Magnet localizer(Magnet &magnet);
    //Localization function
    void computeLocalization();
    //Return current position of localizer
    CImg<double> currentPosition();
    // set save location
    void setFileLocationAndName(QString filename);
    // Save to file
    void saveToFile();
    // Stop saving to file
    void stopSavingToFile();
    //read Raw File
    void readRawFile();
    // set the sensor File
    void setSensorFile(QString readSensorDataFileName);

};
#endif // VIDEOLocalizationThread_H
