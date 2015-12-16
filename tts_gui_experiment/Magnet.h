/*
 * Magnet.h
 *
 * Author: Nordine Sebkhi
 */

#ifndef MAGNET_H_
#define MAGNET_H_

#include <QVector3D>
#include <QVector2D>

class Magnet {

private:
    QVector3D position, moment;
    QVector2D angles;
    double diameter, length, Bt, Br;

public:
    Magnet();
    void setProprieties(double diam, double len, double Bt, double Br);
    void setPosition(float x, float y, float z);
    QVector3D getPosition();
    void setMoment(double theta, double phi);
    QVector2D getAngles();
    QVector3D getTheoField(QVector3D sensorPos);
    void print();

};

#endif /* MAGNET_H_ */
