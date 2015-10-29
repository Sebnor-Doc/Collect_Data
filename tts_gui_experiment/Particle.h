#ifndef PARTICLE_H
#define PARTICLE_H

#include "SensorBoard.h"
#include "Magnet.h"
#include "Sensor.h"
#include <vector>

using namespace std;
using cimg_library::CImg;

class Particle : public Magnet
{
public:
    CImg<double> u_vel, v_vel;
    vector<Sensor> sensors;
    deque<SensorBoard*> sensorboards;
    CImg<double> delta_pos;
    double weight;

public:
    Particle();
    ~Particle();
    Particle(double x, double y, double z, double theta,
             double phi, Magnet m, vector<Sensor> sensors,
             deque<SensorBoard*> sensorboards, double Br, double Bt);
    Particle(boost::shared_ptr<Particle> p);
    double computeWeight(deque<Measurement> calibrated);
    void updatePosition(CImg<double> acc, double dt);
    void updateRotation(double delta_theta, double delta_phi);
};

#endif // PARTICLE_H
