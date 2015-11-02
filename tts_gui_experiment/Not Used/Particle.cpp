#include "particle.h"

Particle::Particle()
{
    this->length(0);
    this->diameter(0);
    this->Br(0);
    this->m_position.assign(1,3).fill(0);
    this->m_moment.assign(1,3).fill(0);
    moment(0, 0);
    this->u_vel.assign(1,3).fill(0);
    this->v_vel.assign(1,3).fill(0);
    this->weight = 0;
}

Particle::~Particle()
{
}

Particle::Particle(double x, double y, double z, double theta, double phi,
                   Magnet m, vector<Sensor> sensors,
                   deque<SensorBoard*> sensorboards, double Br, double Bt)
{
    m_length = m.length();
    m_diameter = m.diameter();
    m_Br = m.Br();
    this->m_position.assign(1,3).fill(x,y,z);
    this->m_moment.assign(1,3).fill(0);
    moment(theta, phi);
    this->u_vel.assign(1,3).fill(0);
    this->v_vel.assign(1,3).fill(0);
    this->sensors = sensors;
    this->sensorboards = sensorboards;
    this->m_Br = Br;
    this->m_Bt = Bt;
    this->delta_pos.assign(1,3).fill(0);
    this->weight = 0;
}

Particle::Particle(boost::shared_ptr<Particle> p)
{
    m_length = p->m_length;
    m_diameter = p->m_diameter;
    moment(p->m_theta,p->m_phi);
    m_position = p->m_position;
    u_vel = p->u_vel;
    v_vel = p->v_vel;
    sensors = p->sensors;
    sensorboards = p->sensorboards;
    m_Br = p->m_Br;
    m_Bt = p->m_Bt;
    delta_pos = p->delta_pos;
    weight = p->weight; // doesnt really matter
}

void Particle::updatePosition(CImg<double> acc, double dt)
{
    // v = u + at
    v_vel = u_vel + acc*dt;
    // S += 1/2*(u+v)*t
    delta_pos = .5*(u_vel + v_vel)*dt;
    m_position += delta_pos;
    u_vel = v_vel;
}
void Particle::updateRotation(double delta_theta, double delta_phi)
{
    m_theta += delta_theta;
    m_phi += delta_phi;
}

double Particle::computeWeight(deque<Measurement> calibrated)
{
    double weight = 0; int i = 1;
    vector<Sensor>::iterator it = sensors.begin();
    vector< CImg<double> >::iterator it2 = calibrated.front().fields.begin();
    for(; it != sensors.end(); ++it, ++it2)
    {
        double t = ((fields((*it).position())-(*it2)).magnitude());
        weight += (t);

//        cout<<"("<<i<<","<<t<<"),";

        i++;
    }
    //if (weight<48)
//        cout<<"Weight "<< weight <<","<< m_position(0)<<","<< m_position(1)<<","<< m_position(2)<<endl<<endl<<endl;

    if (weight<min_wt)
        min_wt = weight;

    return weight;
}






