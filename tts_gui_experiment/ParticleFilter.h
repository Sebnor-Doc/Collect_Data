#ifndef PARTICLEFILTER_H
#define PARTICLEFILTER_H

#include "SensorBoard.h"
#include "constants.h"
#include "Particle.h"
#include <random>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>



using namespace std;

default_random_engine generator;
normal_distribution<double> distribution(NORMAL_DISTRIBUTION_MEAN,NORMAL_DISTRIBUTION_STD);

class ParticleFilter
{
public:
    Magnet m;
    vector<Sensor> sensors;
    deque<SensorBoard*> sensorboards;
    boost::shared_ptr<Particle> particles[NUM_PARTICLES];
    boost::shared_ptr<Particle> weighted_particles[NUM_PARTICLES];
    CImg<double> weights;
    deque<Measurement> calibrated;
    boost::thread run_filter_thread;

public:
    ParticleFilter();
    ParticleFilter(vector<Sensor> sensors, deque<SensorBoard*> sensorboards, Magnet m);
    void RunThread();
    void initializeParticles();
    void runParticleFilter();
    void runParticleFilterSingleIteration();
    void computeWeights();
    void ResampleParticles();
    void updateParticleMotion();
    CImg<double> meanPositionEstimation();
    bool withinRange();
};

#endif // PARTICLEFILTER_H
