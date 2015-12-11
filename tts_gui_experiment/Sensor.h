/*
 * Magnet.h
 *
 *  Created on: Mar 17, 2012
 *      Author: jacob
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <QVector>

class Sensor {

public:
    //////////////////////////////////////
    /// Variables
    /////////////////////////////////////
    unsigned short id;

    QVector<double> position, EMF, offset, gain, angles;


    //////////////////////////////////////
    /// Methods
    /////////////////////////////////////
	Sensor();

//    QVector<int> getCurrentField();
//    void updateCurrentField(int Bx, int By, int Bz);

//	cimg_library::CImg<double> position();
//	void position(cimg_library::CImg<double> position);

//	cimg_library::CImg<double> EMF();
//	void EMF(cimg_library::CImg<double> EMF);

//	cimg_library::CImg<double> offset();
//	void offset(cimg_library::CImg<double> offset);

//	cimg_library::CImg<double> gain();
//	void gain(cimg_library::CImg<double> gain);

//	cimg_library::CImg<double> angles();
//	Sensor& angles(cimg_library::CImg<double> angles);
//    Sensor& angles(double alpha, double beta, double gamma);

//private:
//    cimg_library::CImg<double> rotation();
//    cimg_library::CImg<double> rotationInverse();
//    void print();
};


#endif /* SENSOR_H_ */
