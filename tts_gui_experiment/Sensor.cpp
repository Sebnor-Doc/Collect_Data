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
    this->m_position.assign(1,3).fill(0);
    this->m_EMF.assign(1,3).fill(0);
    this->m_offset.assign(1,3).fill(0);
    this->m_gain.assign(3,3).fill(0);
    this->m_angles.assign(1,3).fill(0);
    this->m_currentField.assign(1,3).fill(0);
    this->m_extrapField.assign(1,3).fill(0);

    v_currentField.fill(0,3);
}


/* ---------------------------------------------------- *
 *                  Calibration                         *
 * ---------------------------------------------------- */

void Sensor::calibrate() {
    m_extrapField =  this->rotation()*this->gain()*(this->currentField() - this->EMF() + this->offset());
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
cimg_library::CImg<int> Sensor::currentField()
{
    return this->m_currentField;
}

void Sensor::currentField(int x, int y, int z){
    qDebug() << "x: " << x << "\ty: " << y << "\tz: " << z;
    v_currentField[0] = x;
    v_currentField[1] = y;
    v_currentField[2] = z;

//    this->m_currentField.fill(x,y,z);
    qDebug() << "current field DONE";
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


//double sensorErrorFunction(unsigned n, const double *x, double *grad, void *my_func_data) {
//    CImg<double> gain;
//    gain.assign(3,3).fill(x[0], 0, 0, 0, x[1], 0, 0, 0, x[2]);
//    CImg<double> sensor_position;
//    sensor_position.assign(1,3).fill(x[3],x[4],x[5]);

//    CImg<double> R(3,3,1,1,0);
//    R = CImg<double>::rotation_matrix(0,0,1,x[6]/180*PI);//Z
//    R = CImg<double>::rotation_matrix(1,0,0,x[7]/180*PI)*R;//X
//    R = CImg<double>::rotation_matrix(0,0,1,x[8]/180*PI)*R;//Z

//    R = R.transpose();

//    CImg<double> EMF;
//    EMF.assign(1,3).fill(x[9], x[10], x[11]);

//    CImg<double> calibrated;

//    double error = 0, temp = 0;

//    for (int i=0; i<calFields.size(); i++)
//    {
//        calibrated = R*gain*(calFields.at(i) - EMF);
//        cal->m_position = theoFieldPositions.at(i);
//        temp = qPow((calibrated - cal->fields(sensor_position)).magnitude(),2);
//        error += temp;
//    }
//    return error;
//}


//void Sensor::calibrateParams(Magnet *cal2) {
//    calFields = m_calFields;
//    theoFieldPositions = m_theoFieldPositions;
//    cal = cal2;

//    for (int i=0; i<m_calFields.size(); i++)
//    {
//        cal->m_position = m_theoFieldPositions.at(i);
//        m_theoFields.push_back(cal->fields(this->m_position));
////        qDebug()<<"Magnet Position: "<<m_theoFieldPositions.at(i)(0)<<", "
////                  << m_theoFieldPositions.at(i)(1)<<", "
////                     << m_theoFieldPositions.at(i)(2)<<endl;
////        qDebug()<<"Sensor Position: "<<m_position(0)<<", "
////                  << m_position(1)<<", "
////                     << m_position(2)<<endl;
////        qDebug()<<"Gauss value: "<<m_theoFields.at(i)(0)<<", "
////                  << m_theoFields.at(i)(1)<<", "
////                     << m_theoFields.at(i)(2)<<endl;
////        qDebug()<<endl;

//    }

//    const int n = 12;
//    // Store in optimization routine
//    double x[n] = {m_gain(0,0),   m_gain(1,1),   m_gain(2,2),
//                   m_position(0), m_position(1), m_position(2),
//                   m_angles(0),   m_angles(1),   m_angles(2),
//                   m_EMF(0),      m_EMF(1),      m_EMF(2)};
//    double ynewlo = 0;
//    double lb[n], ub[n];


//    //gain = 50
//    lb[0] = .9*m_gain(0,0);
//    ub[0] =  1.1*m_gain(0,0);

//    lb[1] = .9*m_gain(1,1);
//    ub[1] =  1.1*m_gain(1,1);

//    lb[2] = .9*m_gain(2,2);
//    ub[2] = 1.1*m_gain(2,2);

//    //position tolerance = 5mm
//    lb[3] = .999*m_position(0);
//    ub[3] = 1.001*m_position(0);

//    lb[4] = .999*m_position(1);
//    ub[4] = 1.001*m_position(1);

//    lb[5] = .999*m_position(2);
//    ub[5] = 1.001*m_position(2);

//    //angles
//    lb[6] = m_angles(0)*0.99;
//    ub[6] = m_angles(0)*1.01;

//    lb[7] = m_angles(1)*0.99;
//    ub[7] = m_angles(1)*1.01;

//    lb[8] = m_angles(2)*0.99;
//    ub[8] = m_angles(2)*1.01;

//    //EMF tolerance
//    lb[9] = -32000;//.95*m_EMF(0);
//    ub[9] = 32000;//1.05*m_EMF(0);

//    lb[10] = -32000;//.95*m_EMF(1);
//    ub[10] = 32000;//1.05*m_EMF(1);

//    lb[11] = -32000;//.95*m_EMF(2);
//    ub[11] = 32000;//1.05*m_EMF(2);

//    //Swap bounds that have got reversed
//    for (int i=0; i<12; i++)
//    {
//        if (lb[i]>ub[i])
//        {
//            double temp = lb[i];
//            lb[i] = ub[i];
//            ub[i] = temp;
//        }
//    }

//    opt = nlopt_create(NLOPT_LN_NELDERMEAD, n);
//    nlopt_set_lower_bounds(opt, lb);
//    nlopt_set_upper_bounds(opt, ub);
//    nlopt_set_min_objective(opt, sensorErrorFunction, NULL);
//    nlopt_set_xtol_rel(opt, 1e-4);
//    qDebug()<<"Optimization returns: "<<
//              nlopt_optimize(opt, x, &ynewlo)
//           <<endl;
//    qDebug()<<"New lowest error: "<< ynewlo << endl;
//    m_gain(0,0) = x[0]; m_gain(1,1) = x[1]; m_gain(2,2) = x[2];
//    m_position(0) = x[3]; m_position(1) = x[4]; m_position(2) = x[5];
//    m_angles(0) = x[6]; m_angles(1) = x[7]; m_angles(2) = x[8];
//    m_EMF(0) = x[9]; m_EMF(1) = x[10]; m_EMF(2) = x[11];

//}

