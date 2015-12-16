/*
 *      Author: Nordine Sebkhi
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <QVector>
#include <QVector3D>

#include<opencv2/core/core.hpp>
using namespace cv;

class Sensor {

public:
    unsigned short id;

    Sensor(unsigned short id,
           QVector<double> pos,
           QVector<double> angles,
           QVector<double> rawGain,
           QVector<double> offset
           );

    void setEMF(QVector<double> emf);
    QVector3D getMagField();
    QVector3D getPosition();
    void updateMagField(int Bx, int By, int Bz);

private:

    QVector<double> angles;

    Matx31d position, magField;
    Matx31d offset, EMF, correction;
    Matx33d gain, rotation, rotGain;

    void setRotation();
    void print();
};


#endif /* SENSOR_H_ */
