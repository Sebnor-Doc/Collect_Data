/*
 * Magnet.h
 *
 *  Created on: Mar 17, 2012
 *      Author: jacob
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "Magnet.h"
#include "CImg.h"
#include "qmath.h"
#include "QVector"

//#include <iostream>
//#include <string>
//#include <boost/lexical_cast.hpp>
//#include "pugixml.hpp"
//#include "common.h"
//#include "asa047.hpp"

class Sensor {

public:
    unsigned int id;
    cimg_library::CImg<int> m_currentField;
    cimg_library::CImg<double> m_extrapField;
    QVector<cimg_library::CImg<double>> m_calFields;
    QVector<cimg_library::CImg<double>> m_theoFields;
    QVector<cimg_library::CImg<double>> m_theoFieldPositions;
    cimg_library::CImg<double> m_position, m_EMF, m_offset, m_gain, m_angles;

    QVector<int> v_currentField;

	Sensor();

    void calibrate();
    cimg_library::CImg<double> calibrate(cimg_library::CImg<double> field);

	cimg_library::CImg<double> position();
	void position(cimg_library::CImg<double> position);

    cimg_library::CImg<int> currentField();
    void currentField(int x, int y, int z);

	cimg_library::CImg<double> EMF();
	void EMF(cimg_library::CImg<double> EMF);

	cimg_library::CImg<double> offset();
	void offset(cimg_library::CImg<double> offset);

	cimg_library::CImg<double> gain();
	void gain(cimg_library::CImg<double> gain);

	cimg_library::CImg<double> rotation();
	cimg_library::CImg<double> rotationInverse();

	cimg_library::CImg<double> angles();    
	Sensor& angles(cimg_library::CImg<double> angles);
    Sensor& angles(double alpha, double beta, double gamma);

    void print();

    //Calibration with Nelder Mead
//    void calibrateParams(Magnet *cal);
};


#endif /* SENSOR_H_ */
