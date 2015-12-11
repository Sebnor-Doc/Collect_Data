/*
 * Magnet.cpp
 *
 *  Created on: Mar 17, 2012
 *      Author: jacob
 */

#include "Sensor.h"
#include "common.h" // for PI
#include <QDebug>

using std::cout;
using std::endl;
using cimg_library::CImg;

Sensor::Sensor() {
    m_position.assign(1,3).fill(0);
    m_EMF.assign(1,3).fill(0);
    m_offset.assign(1,3).fill(0);
    m_gain.assign(3,3).fill(0);
    m_angles.assign(1,3).fill(0);


    m_currentField.assign(1,3).fill(0);
//    m_extrapField.assign(1,3).fill(0);

}


/* ---------------------------------------------------- *
 * Compute actual magnetic field from raw sensor readings *
 * ---------------------------------------------------- */

void Sensor::updateCurrentField(int Bx, int By, int Bz){
    cimg_library::CImg<int> rawMagField;
    rawMagField.assign(1,3).fill(Bx, By, Bz);

    m_currentField =  rotation()*gain()*(rawMagField - m_EMF + m_offset);
}


CImg<double> Sensor::rotation() {
    CImg<double> R(3,3,1,1,0);
    R = CImg<double>::rotation_matrix(0,0,1,m_angles(0)/180*PI);//Z
    R = CImg<double>::rotation_matrix(1,0,0,m_angles(1)/180*PI)*R;//X
    R = CImg<double>::rotation_matrix(0,0,1,m_angles(2)/180*PI)*R;//Z
    return R.transpose();
}

CImg<double> Sensor::rotationInverse() {
    CImg<double> R(3,3,1,1,0);
    R = CImg<double>::rotation_matrix(0,0,1,-m_angles(2)/180*PI);
    R = CImg<double>::rotation_matrix(1,0,0,-m_angles(1)/180*PI)*R;
    R = CImg<double>::rotation_matrix(0,0,1,-m_angles(0)/180*PI)*R;
    return R.transpose();
}


/* ---------------------------------------------------- *
 *                  Getter/Setter                       *
 * ---------------------------------------------------- */

// Position
CImg<double> Sensor::position() {
    return this->m_position;
}

void Sensor::position(CImg<double> position) {
    this->m_position = position;
}

// EMF
CImg<double> Sensor::EMF() {
    return this->m_EMF;
}

void Sensor::EMF(CImg<double> EMF) {
    this->m_EMF = EMF;
}

// Gain
CImg<double> Sensor::gain() {
    return this->m_gain;
}

void Sensor::gain(CImg<double> gain) {
    m_gain = gain;
}

// Offset
cimg_library::CImg<double> Sensor::offset() {
    return this->m_offset;
}

void Sensor::offset(CImg<double> offset) {
    this->m_offset = offset;
}

// Current Field
QVector<int> Sensor::getCurrentField()
{
    QVector<int> magField;
    magField.reserve(3);

    magField.append(m_currentField(0,0));
    magField.append(m_currentField(0,1));
    magField.append(m_currentField(0,2));

    return magField;
}


// Angles
CImg<double> Sensor::angles() {
    return m_angles;
}

Sensor& Sensor::angles(CImg<double> angles) {
    m_angles = angles;
    return *this;
}

Sensor& Sensor::angles(double alpha, double beta, double gamma) {
    m_angles.fill(alpha, beta, gamma);
    return *this;
}


/* ---------------------------------------------------- *
 *                  Other                               *
 * ---------------------------------------------------- */


void Sensor::print() {
    qDebug() << "Sensor[" << (this->id) << "]:" << endl;
    qDebug() << "Sensor.position\t = ["
             << (this->m_position(0)) << ";"
             << (this->m_position(1)) << ";"
             << (this->m_position(2)) << "]" << endl;
    qDebug() << "Sensor.EMF\t = ["
             << (this->m_EMF(0)) << ";"
             << (this->m_EMF(1)) << ";"
             << (this->m_EMF(2)) << "]" << endl;
    qDebug() << "Sensor.offset\t = ["
             << (this->m_offset(0)) << ";"
             << (this->m_offset(1)) << ";"
             << (this->m_offset(2)) << "]" << endl;
    qDebug() << "Sensor.gain\t = [["
             << (this->m_gain(0,0)) << ";"
             << (this->m_gain(1,0)) << ";"
             << (this->m_gain(2,0)) << "]\n\t\t    ["
             << (this->m_gain(0,1)) << ";"
             << (this->m_gain(1,1)) << ";"
             << (this->m_gain(2,1)) << "]\n\t\t    ["
             << (this->m_gain(0,2)) << ";"
             << (this->m_gain(1,2)) << ";"
             << (this->m_gain(2,2)) << "]]" << endl;
    qDebug() << "Sensor.angles\t = [" << (this->m_angles(0))<<","
             <<(this->m_angles(1))<< ","
            << (this->m_angles(2)) << "]" << endl;
    qDebug() << endl;
}


