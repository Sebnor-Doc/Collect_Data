/*
 * Magnet.h
 *
 * Author: Nordine Sebkhi
 */

#ifndef MAGNET_H_
#define MAGNET_H_

#include <QVector3D>

class Magnet {

private:
    QVector3D position, moment;
    double diameter, length, Bt, Br;

public:
    Magnet();
    void setProprieties(double diam, double len, double Bt, double Br);
    void setPosition(float x, float y, float z);
    void setMoment(double theta, double phi);
    QVector3D getTheoField(QVector3D sensorPos);
    void print();

};

#endif /* MAGNET_H_ */
