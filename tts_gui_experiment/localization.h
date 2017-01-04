#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QVector>

#include "common.h"
#include "Sensor.h"
#include "Magnet.h"

#include <opencv2/core/optim.hpp>
#include <opencv2/core/core.hpp>
using namespace cv;




Q_DECLARE_METATYPE(LocaData)
const int dontcare3 = qRegisterMetaType<LocaData>();


class Localizer : public MinProblemSolver::Function, public QObject {

public:
    Localizer(QObject *parent = 0);
    void init(QVector<Sensor*> sensors, Magnet *magnetPtr);
    void setMagData(MagData& magData);
    double calc (const double *x) const;
    int getDims() const;

private:
    QVector<Sensor*> sensors;
    Magnet* magnet;
    MagData magData;
};

class LocaWorker : public QObject
{
    Q_OBJECT

public:
    LocaWorker(QObject *parent = 0);
    void init(QVector<Sensor*> sensors, Magnet &magnet);

public slots:
    void start();
    void localize(MagData origData, QString filename);

signals:
    void dataLocalized(LocaData localizedData);

private:
    Magnet magnet;
    Ptr<DownhillSolver> nealderMead;
    Ptr<Localizer> localizer;
};

class Localization : public QObject
{
    Q_OBJECT

public:
    Localization(QObject *parent = 0);
    void init(QVector<Sensor *> sensors, Magnet &magnet);

public slots:
    void start();
    void setFilename(QString fileRoot);
    void processMag(MagData magData);
    void processLoca(LocaData locaData);

signals:
    void packetToLoca(MagData magData, QString filename);
    void packetLocalized(LocaData);

private:
    LocaWorker locaWorker;
    QString filename;
    QFile outputFile;
    QTextStream outputStream;
};

#endif // LOCALIZATION_H
