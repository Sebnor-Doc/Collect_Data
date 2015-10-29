#ifndef ReadSensors_H
#define ReadSensors_H

#include "mojoserialport.h"
#include "Sensor.h"
#include "CImg.h"

#include <QFile>
#include <QWaitCondition>
#include <QMutex>
#include <QThread>
#include <QTextStream>


class ReadSensors: public QThread
{    Q_OBJECT

 private:
    bool stop;
    QWaitCondition condition;
    bool readSensors;
    QMutex mutex;
    cimg_library::CImg<double> currentField;
    MOJOSerialPort *sp;
    QString source;
    bool save;    
    QFile sensorOutputFile;
    QTextStream *sensorOutputFile_stream;

protected:
     void run();
     void msleep(int ms);

 public:
    QString filename;

    ReadSensors(QObject *parent = 0);
    ~ReadSensors();

    void Play();            // Run the webcam
    void Stop();            // Stop

    bool isStopped() const; // Check if the ReadSensors has been stopped
    void beginRecording();  // Begin Recording
    void endRecording();    // End Recording

    cimg_library::CImg<double> getCurrentField();   // Get Current field

    bool checkCOMPorts();   // Check if COM Port is correctly outputting data
    void setCOMPort(QString source);    // Set Source (COM Port)

    short getSensorData(int pcb, int sensor, int dim); // Send specific sensor data

    void setFileLocationAndName(QString filename); // Set save location

    void saveToFile();  // Save to file

    void stopSavingToFile();    // Stop saving to file

};
#endif // ReadSensors_H
