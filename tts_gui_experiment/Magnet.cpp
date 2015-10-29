/*
 * Magnet.cpp
 *
 *  Created on: Mar 17, 2012
 *      Author: jacob
 */

#include "Magnet.h"
#include <QDebug>

using std::string;
using std::cout;
using std::endl;
using namespace cimg_library;
//using cimg_library::CImg;
//using boost::lexical_cast;

Magnet::Magnet() {
	this->length(0);
	this->diameter(0);
	this->Br(0);
	this->m_position.assign(1,3).fill(0);
	this->m_moment.assign(1,3).fill(0);
	moment(0, 0);
}

CImg<double> Magnet::fields(const CImg<double> position) {
    CImg<double> R = position - m_position;
    double R_Mag = R.magnitude();

    return this->Bt()*(3*this->moment().dot(R)*(R)/pow(R_Mag,5)-this->moment()/pow(R_Mag,3));
}

/*********
 * Setter
 *********/
void Magnet::diameter( double diameter) {
    this->m_diameter = diameter;
}

void Magnet::length( double length ) {
    this->m_length = length;
}

void Magnet::Br( double Br ) {
    this->m_Br = Br;
}

void Magnet::Bt( double Bt) {
    this->m_Bt = Bt;
}

Magnet& Magnet::position(CImg<double> position) {
    this->m_position = position;
    return *this;
}

Magnet& Magnet::moment(double theta, double phi) {
    this->m_theta = theta;
    this->m_phi = phi;
    this->m_moment.fill(sin(this->m_theta)*cos(this->m_phi),
                    sin(this->m_theta)*sin(this->m_phi),
                    cos(this->m_theta));
    return *this;
}

Magnet& Magnet::moment(cimg_library::CImg<double> moment) {
    m_moment = moment/=moment.magnitude();
    m_theta = atan2(m_moment(1),m_moment(0));
    m_phi = acos(m_moment(2));
    return *this;
}

void Magnet::theta(double newTheta) {
    moment(newTheta, this->m_phi);
}

void Magnet::phi(double newPhi) {
    moment(this->m_theta, newPhi);
}

/*********
 * Getter
 *********/
double Magnet::diameter() {
	return this->m_diameter;

}

double Magnet::length() {
	return this->m_length;
}

double Magnet::Br() {
	return this->m_Br;
}

double Magnet::Bt() {
	return this->m_Bt;
}

CImg<double> Magnet::position() {
	return this->m_position;
}

CImg<double> Magnet::moment() {
	return this->m_moment;
}

double Magnet::theta() {
	return this->m_theta;
}

double Magnet::phi() {
	return this->m_phi;
}

/*********
 * Print
 *********/
void Magnet::print() {
    qDebug() << "Magnet:" << endl;
    qDebug() << "Magnet.diameter\t = " << (this->diameter()) << endl;
    qDebug() << "Magnet.length\t = " <<(this->length()) << endl;
    qDebug() << "Magnet.Br\t = " << (this->Br()) << endl;
    qDebug() << "Magnet.Bt\t = " << (this->Bt()) << endl;
    qDebug() << "Magnet.theta\t = " << (this->theta()) << endl;
    qDebug() << "Magnet.phi\t = " << (this->phi()) << endl;
    qDebug() << "Magnet.moment\t = ["
         << (this->moment()(0)) << ";"
         << (this->moment()(1)) << ";"
         << (this->moment()(2)) << "]" << endl;
    qDebug() << "Magnet.position\t = ["
         << (this->position()(0)) << ";"
         << (this->position()(1)) << ";"
         << (this->position()(2)) << "]" << endl;
    qDebug() << endl;
}

string Magnet::printVector(const CImg<double>& myVector) {
	int width = myVector.width();
	int height = myVector.height();
	string output;

	for (int i = 0; i < width; ++i) {
		output.append("[");
		for (int j = 0; j < height; ++j) {
            output.append(boost::lexical_cast<string>(myVector(i,j)));
			output.append(";");
		}
		output.append("]\n");
	}

	return output;
}
