#include "Sensor.h"
#include <qmath.h>
#include <QDebug>

using std::endl;


Sensor::Sensor(unsigned short id, QVector<double> pos, QVector<double> angles, QVector<double> rawGain, QVector<double> offset) {

    this->id = id;

    // Initialize 3x3 matrices
    gain = gain.mat_type::zeros();
    EMF = EMF.mat_type::zeros();
    correction = correction.mat_type::zeros();
    rotGain = rotGain.mat_type::zeros();

    // Set matrices
    for (int i = 0; i < 3; i++) {
        this->position(i) = pos.at(i);
        this->offset(i) = offset.at(i);
        gain(i,i) = rawGain.at(i);
        magField(i) = 0.0;
    }

    this->angles = angles;
    setRotation();
}

void Sensor::updateMagField(int Bx, int By, int Bz){

    Matx31d origField(Bx, By, Bz);
    magField =  rotGain * (origField + correction);
}

QVector3D Sensor::getMagField()
{
    return QVector3D(magField(0), magField(1), magField(2));
}

QVector3D Sensor::getPosition() {
    return QVector3D(position(0), position(1), position(2));
}

void Sensor::setRotation(){
    // Convert euler angles from degrees to radians
    double alpha    = qDegreesToRadians(angles.at(0));
    double beta     = qDegreesToRadians(angles.at(1));
    double gamma    = qDegreesToRadians(angles.at(2));

    // Compute the rotation matrix using sensor angles relative to TTS reference
    rotation(0,0) = qCos(gamma)*qCos(alpha) - qCos(beta)*qSin(alpha)*qSin(gamma);
    rotation(0,1) = qCos(gamma)*qSin(alpha) + qCos(beta)*qCos(alpha)*qSin(gamma);
    rotation(0,2) = qSin(gamma)*qSin(beta);
    rotation(1,0) = -qSin(gamma)*qCos(alpha) - qCos(beta)*qSin(alpha)*qCos(gamma);
    rotation(1,1) = -qSin(gamma)*qSin(alpha) + qCos(beta)*qCos(alpha)*qCos(gamma);
    rotation(1,2) = qCos(gamma)*qSin(beta);
    rotation(2,0) = qSin(beta)*qSin(alpha);
    rotation(2,1) = -qSin(beta)*qCos(alpha);
    rotation(2,2) = qCos(beta);

    // Get inverse matrix to get rotation from Sensor to TTS reference
    rotation = rotation.inv();

    // Update Rotation*Gain Matrix
    rotGain = rotation * gain;
}

void Sensor::setEMF(QVector<double> emf){

    for (int i = 0; i < 3; i++) {
        this->EMF(i) = emf.at(i);
    }

    // Update Correction matrix
    correction = offset - EMF;
}

void Sensor::print() {
    qDebug() << "Sensor[" << id << "]:" << endl;

    qDebug() << "Sensor.position\t = ["
             << position(0) << ";"
             << position(1) << ";"
             << position(2) << "]" << endl;

    qDebug() << "Sensor.angles\t = ["
             << angles.at(0) << ","
             << angles.at(1)<< ","
            <<  angles.at(2) << "]" << endl;

    qDebug() << "Sensor.gain\t = [["
             << gain(0,0) << ";"
             << gain(0,1) << ";"
             << gain(0,2) << "]\n\t\t    ["
             << gain(1,0) << ";"
             << gain(1,1) << ";"
             << gain(1,2) << "]\n\t\t    ["
             << gain(2,0) << ";"
             << gain(2,1) << ";"
             << gain(2,2) << "]]" << endl;

    qDebug() << "Sensor.offset\t = ["
             << offset(0) << ";"
             << offset(1) << ";"
             << offset(2) << "]" << endl;

    qDebug() << "Sensor.Correction\t = ["
             << correction(0) << ";"
             << correction(1) << ";"
             << correction(2) << "]" << endl;

    qDebug() << "Sensor.rotGain\t = [["
             << rotGain(0,0) << ";"
             << rotGain(0,1) << ";"
             << rotGain(0,2) << "]\n\t\t    ["
             << rotGain(1,0) << ";"
             << rotGain(1,1) << ";"
             << rotGain(1,2) << "]\n\t\t    ["
             << rotGain(2,0) << ";"
             << rotGain(2,1) << ";"
             << rotGain(2,2) << "]]" << endl;

    qDebug() << "Sensor.EMF\t = ["
             << EMF(0) << ";"
             << EMF(1) << ";"
             << EMF(2) << "]" << endl;

    qDebug() << endl;
}
