#include "particlefilter.h"

using std::vector;
using std::deque;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::ios;
using boost::mutex;
using boost::thread;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;
using boost::posix_time::seconds;
using boost::lexical_cast;
using namespace cimg_library;

ParticleFilter::ParticleFilter()
{

}

ParticleFilter::ParticleFilter(vector<Sensor> sensors, deque<SensorBoard*> sensorboards, Magnet m)
{
    this->sensors = sensors;
    this->sensorboards = sensorboards;
    this->m = m;
    weights.assign(1,NUM_PARTICLES).fill(0);
    initializeParticles();
}

void ParticleFilter::RunThread()
{
    run_filter_thread = thread(bind(&ParticleFilter::runParticleFilter, this));
}

void ParticleFilter::runParticleFilter()
{
    while (true)
    {
        updateParticleMotion();
        computeWeights();
        ResampleParticles();
    }
}

void ParticleFilter::runParticleFilterSingleIteration()
{
    updateParticleMotion();
    computeWeights();
    ResampleParticles();
}

void ParticleFilter::initializeParticles()
{
    for (int i=0; i<NUM_PARTICLES; i++)
    {
        particles[i] = boost::shared_ptr<Particle>(new Particle(roll(-RANGE,RANGE),roll(-0.03,RANGE),roll(-RANGE,RANGE),roll(-PI/2,PI/2),roll(-PI,PI),m, sensors, sensorboards,m.m_Br, m.m_Bt));
    }

}

void ParticleFilter::updateParticleMotion()
{
    for (int i=0;i<NUM_PARTICLES;i++)
    {
        CImg<double> acc;
        acc.assign(1,3).fill(roll(-1,1), roll(-1,1),roll(-1,1));
        particles[i]->updatePosition(acc,.025);

        //        acc.assign(1,3).fill(roll(-0.01,0.01), roll(-0.01,0.01),roll(-0.01,0.01));
        //        particles[i]->m_position += acc*1;

        double delta_theta = roll(-PI/2,PI/2);
        double delta_phi = roll(-PI,PI);
        particles[i]->updateRotation(0.15*delta_theta, 0.15*delta_phi);
    }

}

void ParticleFilter::computeWeights()
{
    // Update magnetic fields from magnet
    deque<SensorBoard*>::iterator it;
    Measurement combineMeasurements;
    for(it=sensorboards.begin();it!=sensorboards.end();++it) {
        combineMeasurements+=(*it)->data(1)[0];
    }
    calibrated.push_front(combineMeasurements.calibrate(sensors));

    double norm=0;
    for (int i=0; i<NUM_PARTICLES; i++)
    {
        weights(i) = particles[i]->computeWeight(calibrated);
        weights(i) = weights(i) - min_wt + 0.01;
        weights(i) = 1/(weights(i));
        norm += weights(i)*weights(i);
    }
    norm = sqrt(norm);
    weights = weights/norm;

    //    for (int i=0; i<NUM_PARTICLES; i++)
    //    {
    //        particles[i]->weight = weights(i);
    //    }
}

void ParticleFilter::ResampleParticles()
{
    double beta = 0;

    int index = ((int)roll(0,NUM_PARTICLES))%NUM_PARTICLES;

    double weights_max = 0;
    for (int i=0;i<NUM_PARTICLES;i++)
    {
        if (weights(i)>weights_max)
            weights_max = weights(i);
    }

    for (int i=0;i<NUM_PARTICLES;i++)
    {
        beta += roll(1,2*weights_max);

        while (beta>weights(index))
        {
            beta -= weights(index);
            index = (index+1)%NUM_PARTICLES;
        }

        weighted_particles[i] = boost::shared_ptr<Particle>(new Particle(particles[index]));
    }

    for (int i=0;i<NUM_PARTICLES;i++)
    {
        particles[i] = boost::shared_ptr<Particle>(weighted_particles[i]);
    }
}

CImg<double> ParticleFilter::meanPositionEstimation()
{
    CImg<double> sum;
    double avg_theta =0, avg_phi = 0;
    sum.assign(1,3).fill(0);

    for (int i=0; i<NUM_PARTICLES;i++)
    {
        sum += particles[i]->m_position;
        avg_theta += particles[i]->m_theta;
        avg_phi += particles[i]->m_phi;
    }

    return (((CImg<double>()).assign(1,5).fill(sum(0),sum(1),sum(2),avg_theta,avg_phi))/NUM_PARTICLES);
}

bool ParticleFilter::withinRange()
{
    CImg<double> mp = meanPositionEstimation();
    if (abs(mp(0))>RANGE || abs(mp(1)>RANGE || abs(mp(2))>RANGE))
        return false;
    else
        return true;

}



