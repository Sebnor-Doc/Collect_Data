#ifndef ReadSensors_H
#define ReadSensors_H

#include "mojoserialport.h"
#include "Sensor.h"
#include "common.h"
#include "CImg.h"

#include <QFile>
#include <QWaitCondition>
#include <QMutex>
#include <QThread>
#include <QTextStream>

#include <QVector>

class ReadSensors: public QThread
{    Q_OBJECT

private:
    QWaitCondition condition;
    QMutex mutex;

    bool stop;
    bool save;

    MOJOSerialPort *sp;
    QString source;

    QString filename;
    QFile sensorOutputFile;
    QTextStream *sensorOutputFile_stream;


protected:
     void run();

signals:
   void newPacketAvail(MagData *packet);

private slots:
    void processPacket(MagData *packet);

public:
    ReadSensors(QObject *parent = 0);
    ~ReadSensors();

    void Play();
    void Stop();

    void beginRecording();

    short getSensorData(int pcb, int sensor, int dim);

    void setFileLocation(QString filename);
    void saveToFile();
    void stopSavingToFile();


private:
    bool isStopped() const;
    void setCOMPort(QString source);
    bool checkCOMPorts();

};
#endif // ReadSensors_H
