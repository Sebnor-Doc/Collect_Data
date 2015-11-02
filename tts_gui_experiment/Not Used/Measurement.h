/*
 * Measurement.h
 *
 *  Created on: Apr 7, 2012
 *      Author: jacob
 */

#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

#include <vector>
#include <string>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "CImg.h"
#include "Sensor.h"

class Measurement {
private:
	int m_id;
	std::string m_time; /// Date string of the measurement
	cimg_library::CImg<double> m_position; /// Position of the robotic arm, if available.

public:
	Measurement calibrate(std::vector<Sensor>& sensors);
	void print();
	Measurement();
	Measurement(std::string input, bool boolPosition, bool print = false);
	Measurement(std::vector<int> input, bool boolPosition = false, bool print=false);
	Measurement(Measurement &measurement1,Measurement &measurement2);
	Measurement& operator+=(Measurement &measurement);
	Measurement operator+(Measurement &measurement);
	std::string time();
	void time(std::string time);
	int id();
	void id(int id);
	cimg_library::CImg<double> position();
	void position(cimg_library::CImg<double>);
	std::vector< cimg_library::CImg<double> > fields; /// Measurements from the sensor boards.
	virtual ~Measurement();
};

#endif /* MEASUREMENT_H_ */
