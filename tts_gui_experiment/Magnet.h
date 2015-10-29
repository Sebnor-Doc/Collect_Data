/*
 * Magnet.h
 *
 *  Created on: Mar 17, 2012
 *      Author: jacob
 */

#ifndef MAGNET_H_
#define MAGNET_H_

#include <iostream>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include "CImg.h"

class Magnet {

public:
	double m_theta, m_phi, m_length, m_diameter, m_Bt, m_Br;
	cimg_library::CImg<double> m_moment, m_position;
	std::string printVector(const cimg_library::CImg<double>& myVector);

public:
	Magnet();

	cimg_library::CImg<double> position();
	Magnet& position(cimg_library::CImg<double> position);

	cimg_library::CImg<double> moment();
	Magnet& moment(double theta, double phi);
	Magnet& moment(cimg_library::CImg<double> moment);

	double diameter();
	void diameter( double diameter);
	double Br();
	void Br( double Br );
	double Bt();
	void Bt( double Bt ); //TODO Can calculate Bt for specific shapes.
	double length();
	void length( double length );
	double theta();
	void theta( double theta);
	double phi();
	void phi( double phi );
	void print();

	cimg_library::CImg<double> fields(const cimg_library::CImg<double> position);
	//	~Magnet();
};

#endif /* MAGNET_H_ */
