/*
 * Measurement.cpp
 *
 *  Created on: Apr 7, 2012
 *      Author: jacob
 */

#include "Measurement.h"

using std::string;
using std::vector;
using std::cout;
using std::endl;
using boost::lexical_cast;
using boost::tokenizer;
using boost::escaped_list_separator;
using cimg_library::CImg;

Measurement::Measurement() {
}

Measurement::Measurement(string input, bool boolPosition, bool print) {
    if (print) cout << input;

    escaped_list_separator<char> els(""," \n\r","");
    tokenizer< escaped_list_separator<char> > tok(input, els);

    vector<string> myInput(tok.begin(),tok.end());
    vector<string>::iterator it = myInput.begin();
    this->m_time = *it++; // Date
    this->m_time.append(" ").append(*it++); // Time HH:MM:SS
    this->m_time.append(" ").append(*it++); // Time AM/PM
    this->m_position = CImg<double>(1,3); // Load robotic arm position information if indicated.
    if (boolPosition) {
        this->m_position(0) = boost::lexical_cast<double>(*it++);
        this->m_position(1) = boost::lexical_cast<double>(*it++);
        this->m_position(2) = boost::lexical_cast<double>(*it++);
    }
    this->m_position*=.01; //TODO everything should be meters inside and out.
    // Load the remainder of data as sensor field data.
    while(it != myInput.end()-1) {
        CImg<double> myVector(1,3);
        myVector(0) = lexical_cast<double>(*it++);
        myVector(1) = lexical_cast<double>(*it++);
        myVector(2) = lexical_cast<double>(*it++);
        this->fields.push_back(myVector);
    }

    if (print) 	this->print();
}

//TODO: remove print from argument?
Measurement::Measurement(vector<int> input, bool boolPosition, bool print) {
    vector<int>::iterator it = input.begin();

    if (boolPosition) {
        m_position = CImg<double>(1,3);
        m_position(0) = *(it++);
        m_position(1) = *(it++);
        m_position(2) = *(it++);
    }

    while(it!=input.end()) {
        fields.push_back(CImg<double>(1,3));
        fields.back()(0) = *(it++);//x
        fields.back()(1) = *(it++);//y
        fields.back()(2) = *(it++);//z
    }
}

Measurement::Measurement(Measurement &measurement1,Measurement &measurement2) {
    (*this) = measurement1;
    this->fields.insert(this->fields.end(),measurement2.fields.begin(),measurement2.fields.end());
}

Measurement& Measurement::operator+=(Measurement &measurement) {
    (*this) = Measurement(*this,measurement);
    return *this;
}

Measurement Measurement::operator+ (Measurement &measurement) {
    return Measurement(*this,measurement);
}

Measurement::~Measurement() {
    // TODO Auto-generated destructor stub
}

Measurement Measurement::calibrate(vector<Sensor>& sensors) {
    Measurement calibrated(*this);//doesn't do anything
    calibrated.fields.clear();
    vector< Sensor >::iterator it = sensors.begin();
    vector< CImg<double> >::iterator it2 = this->fields.begin();
    for (;it != sensors.end() ; ++it, ++it2 ) {
        //calibrated.fields.push_back((*it).calibrate(*it2));
    }

    return calibrated;
}

void Measurement::print() {
    cout << "Measurement:" << endl;
    cout << "Measurement.time\t = " << lexical_cast<string>(this->m_time) << endl;
    if (!this->m_position.is_empty()) {
        cout << "Measurement.position\t = [" << lexical_cast<string>(this->m_position(0)) <<  ";"
                                             << lexical_cast<string>(this->m_position(1)) <<  ";"
                                             << lexical_cast<string>(this->m_position(2)) <<  ";]" << endl;
    }
    vector< CImg<double> >::iterator it = this->fields.begin();
    for (; it != this->fields.end(); ++it) {
        cout << "Measurement.fields[" << lexical_cast<string>(it - this->fields.begin()) << "]\t = ["
                                      << lexical_cast<string>((*it)(0)) << ";"
                                      << lexical_cast<string>((*it)(1)) << ";"
                                      << lexical_cast<string>((*it)(2)) << ";]" << endl;
    }
    cout << endl;
}

int Measurement::id() {
    return this->m_id;
}

void Measurement::id(int id) {
    m_id = id;
}

string Measurement::time() {
    return m_time;
}

void Measurement::time(string time) {
    m_time = time;
}

CImg<double> Measurement::position() {
    return m_position;
}
void Measurement::position(CImg<double> position) {
    this->m_position = position;
}
