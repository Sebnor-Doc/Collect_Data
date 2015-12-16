/*
 * Magnet.cpp
 *
 * Author: Nordine Sebkhi
 */

#include "Magnet.h"
#include <QDebug>
#include <qmath.h>

using std::endl;


Magnet::Magnet() {
    // Initialize 3D vectors
    position.setX(0);
    position.setY(0);
    position.setZ(0);

    moment.setX(0);
    moment.setY(0);
    moment.setZ(0);

    angles.setX(0);
    angles.setY(0);
}

void Magnet::setProprieties(double diam, double len, double Bt, double Br)
{
    this->diameter = diam;
    this->length = len;
    this->Bt = Bt;
    this->Br = Br;
}


void Magnet::setPosition(float x, float y, float z)
{
    position.setX( x );
    position.setY( y );
    position.setZ( z );
}

QVector3D Magnet::getPosition()
{
    return position;
}

void Magnet::setMoment(double theta, double phi)
{
    angles.setX(theta);
    angles.setY(phi);

    moment.setX( qSin(theta) * qCos(phi) );
    moment.setY( qSin(theta) * qSin(phi) );
    moment.setZ( qCos(theta) );
}

QVector2D Magnet::getAngles()
{
    return angles;
}

QVector3D Magnet::getTheoField(QVector3D sensorPos)
{
    QVector3D r = sensorPos - position;
    float r_mag = r.length();

    return Bt * ( (3 * QVector3D::dotProduct(moment, r) * r) - (qPow(r_mag, 2) * moment) ) / qPow(r_mag, 5);
}



//CImg<double> Magnet::fields(const CImg<double> position) {
//    CImg<double> R = position - m_position;
//    double R_Mag = R.magnitude();

//    return this->Bt()*(3*this->moment().dot(R)*(R)/pow(R_Mag,5)-this->moment()/pow(R_Mag,3));
//}



//Magnet& Magnet::moment(cimg_library::CImg<double> moment) {
//    m_moment = moment/=moment.magnitude();
//    m_theta = atan2(m_moment(1),m_moment(0));
//    m_phi = acos(m_moment(2));
//    return *this;
//}


/*********
 * Print
 *********/
void Magnet::print() {
    qDebug() << "Magnet:" << endl;

    qDebug() << "Diameter\t = " << diameter << endl;
    qDebug() << "Length\t = " << length << endl;
    qDebug() << "Br\t = " << Br << endl;
    qDebug() << "Bt\t = " << Bt << endl;

//    qDebug() << "Theta\t = " <<  << endl;
//    qDebug() << "Phi\t = " <<  << endl;
    qDebug() << "Moment\t = ["
         << moment.x() << ";"
         << moment.y() << ";"
         << moment.z() << "]" << endl;

    qDebug() << "Position\t = ["
             << position.x() << ";"
             << position.y() << ";"
             << position.z() << "]" << endl;

    qDebug() << endl;
}
