#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <QMutex>
#include <QThread>
#include <QWaitCondition>

#include "ReadSensors.h"
#include "common.h"
#include "Sensor.h"
#include "Magnet.h"

#include <QVector>
#include <vector>

//#include <QImage>
//#include <QTimer>

#include "CImg.h"
using namespace cimg_library;
using std::vector;


class Localization: public QThread
{
    Q_OBJECT

private:
    // Thread status
    bool stop;
    bool save;
    QMutex mutex;



    // Sensor and magnet
    ReadSensors *rs;
    Magnet magnet;
    QVector<Sensor*> sensors;

    // Save localized data
    QString filename;
    QFile outputFile;
    QTextStream *outputFile_stream;


 protected:
     void run();

private slots:
     void computeLocalization(MagData magData);

 public:
    Localization(ReadSensors *rs, QVector<Sensor*> &sensors, Magnet &magnet, QObject *parent = 0);
    ~Localization();

    // Manage thread status
    void Stop();

    // Localization of mag data
    Magnet localizer(Magnet &magnet);

    // Save localized data
    void setFileLocation(QString filename);
    void startSaving();
    void stopSaving();

};
#endif // Localization_H
