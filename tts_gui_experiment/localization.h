#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QTextStream>

#include "common.h"
#include "Sensor.h"
#include "Magnet.h"

struct LocaPoint {
    double x, y, z, theta, phi;
};

struct LocaData {
    MagData magData;
    LocaPoint locaPoint;
    QString filename;
};

Q_DECLARE_METATYPE(LocaData)
const int dontcare3 = qRegisterMetaType<LocaData>();

class LocaWorker : public QObject
{
    Q_OBJECT

public:
    void init(QVector<Sensor*> &sensors, Magnet &magnet);

public slots:
    void start();
    void localize(LocaData origData);

signals:
    void dataLocalized(LocaData localizedData);

private:
    QVector<Sensor*> sensors;
    Magnet magnet;
};

class Localization : public QObject
{
    Q_OBJECT

public:
    void init(QVector<Sensor*> &sensors, Magnet &magnet);

public slots:
    void start();
    void setFilename(QString fileRoot);
    void processMag(MagData magData);
    void processLoca(LocaData locaData);

signals:
    void packetToLoca(LocaData locaData);

private:
    LocaWorker locaWorker;
    QString filename;
    QFile outputFile;
    QTextStream outputStream;
};

#endif // LOCALIZATION_H
