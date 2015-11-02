#ifndef ROBOTICARMWRAPPER_H
#define ROBOTICARMWRAPPER_H

#include <QSerialPort>
#include "common.h"
#include "ReadSensors.h"
#include <QFile>
#include <QObject>

class roboticArmWrapper: public QThread
{Q_OBJECT
public:
    roboticArmWrapper(ReadSensors *rs, QString COM);
    double posX;
    double posY;
    double posZ;
    ReadSensors *rs;

    QString filename;
    QString traj_file;

    QTextStream *rw_pos_raw_stream;

public:
    void saveTo(QString filename);
    void moveTo(double x, double y, double z);
    void saveToFile();
    bool parseJSON(QString t);
    void moveTrajectory(QString traj_file);

public slots:
    void process();

signals:
    void finished();
    void error(QString err);

};

#endif // ROBOTICARMWRAPPER_H
