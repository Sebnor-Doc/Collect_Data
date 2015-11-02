/*
 * SensorBoard.cpp
 *
 *  Created on: Apr 26, 2012
 *      Author: jacob
 */

#include "SensorBoard.h"

using std::vector;
using std::deque;
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::ios;
using boost::mutex;
using boost::bind;
using boost::thread;
using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;
using boost::posix_time::seconds;
using boost::lexical_cast;
using namespace cimg_library;

boost::timer::cpu_timer myClock;
double time_old, time_new;
vector<int> output(48,0);


SensorBoard::SensorBoard(const std::string& name, const unsigned int sensorNumber, const bool handshake, const string fileOutputName, const unsigned int timeout)
    : m_name(name),  m_fileOutputName(fileOutputName), m_channel(sensorNumber), m_timeout(timeout), m_display_running(false) {
    try {
        //        this->open(m_name, baud_rate);
        //        this->setTimeout(seconds(timeout));
        //        if (handshake) this->handshake();
    } catch(boost::system::system_error& e) {
        cout << "Something went wrong setting up " << m_name << ". Error: " << e.what() << endl;
    } catch (...) {
        cout << "Something went wrong setting up " << m_name << endl;
    }


    m_fileOutput.open(m_fileOutputName.c_str());
    cout<<m_fileOutputName.c_str()<<endl;

    time_old = ::atof((myClock.format(5,"%ws")).c_str());
    m_threadRead = thread(bind(&SensorBoard::readBoard, this));


}

SensorBoard::~SensorBoard() {
    cout << "Closing board " << m_name << endl;
    m_threadRead.interrupt();
    m_display_thread.interrupt(); //TODO: It will probably throw an exception...d oh.
    m_fileOutput.close();
    this->close();
    // TODO Auto-generated destructor stub
}

bool SensorBoard::handshake() {
    return true;
}

void SensorBoard::readBoard() {    
    int out;
    int packetcheck = 0;

    for (;;) {
        try {
            //            time_new = ::atof((myClock.format(5,"%ws")).c_str());
            //            if (time_new-time_old<0.008) //sample a little faster than 100 Hz
            //            {
            //                continue;
            //            }
            //            time_old = time_new;

            LONG len = EXPECTED_BYTES;
            //BulkInPipe4->XferData(buf, len);
            if (len != EXPECTED_BYTES)
                continue;

            boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
            m_fileOutput << now.date() << " ";
            {
                mutex::scoped_lock lock(m_mutexRead);
                for(int i = 2; i<98; i+=2) {
                    out = (int16_t) (buf[i]|(buf[i+1]<<8));
                    output.at(i/2-1) = out;
                    m_fileOutput << (out) << " ";
                }
                m_data.push_front(Measurement(vector<int>(output.begin(),output.end())));
                if (m_data.size() > 1024) { m_data.pop_back(); } //TODO: How many samples to keep around?
            }
            m_fileOutput << endl;

        } catch (...) {
            cout << "Read failure, nothing returned from board " << m_name << endl;
        }
    }
}

deque< Measurement > SensorBoard::data(int size) {
    mutex::scoped_lock lock(m_mutexRead);
    if (size>m_data.size()) {
        size = m_data.size();
    }

    return deque< Measurement >(m_data.begin(),m_data.begin()+size);
}

string SensorBoard::name() {
    return m_name;
}

double SensorBoard::battery() {
    return m_battery;
}

void SensorBoard::display() {
    {
        mutex::scoped_lock lock(m_display_mutex);
        m_display_running = !m_display_running;
    }
    if(m_display_running)
        m_display_thread = thread(bind(&SensorBoard::m_display,this));
    else
        m_display_thread.join();
}

void SensorBoard::m_display() {
    const unsigned char red[] = { 255,0,0 }, green[] = { 0,255,0 }, blue[] = { 0,0,255 }, white[] = {255, 255, 255 };
    CImg<double> sensor_img(2000,1000,1,3,0);
    CImgDisplay sensor_disp(sensor_img,"Sensor Readings");

    CImgList<double> graphs(NUM_SENSORS,600,100,1,3,255);
    CImgList<double> traces(NUM_SENSORS,500,3,1,1,0);
    deque< Measurement > data;
    while (!sensor_disp.is_closed()) {
        {
            mutex::scoped_lock lock(m_display_mutex);
            if (!m_display_running)
                return;
        }

        data = this->data(500);
        deque< Measurement >::reverse_iterator data_it;
        cimglist_for(traces,i) {

            traces[i].fill(0);
            data_it = data.rbegin();
            cimg_forX(traces[i],x) {
                if (data_it != data.rend()) {
                    traces[i](x,0) = (*data_it).fields[i](0);
                    traces[i](x,1) = (*data_it).fields[i](1);
                    traces[i](x,2) = (*data_it).fields[i](2);
                    ++data_it;
                }
            }
        }

        sensor_img.fill(0).draw_text(0,0,this->name().c_str(),white);
        sensor_img.draw_line(5,20,15,20,red,1,~0).draw_text(20,13,"x",white);
        sensor_img.draw_line(5,35,15,35,blue,1,~0).draw_text(20,28,"y",white);
        sensor_img.draw_line(5,50,15,50,green,1,~0).draw_text(20,43,"z",white);
        cimglist_for(graphs,i) {
            graphs[i].fill(0).draw_grid(10,10,0,0,false, false, white,0.1,~0U,~0U);
            int ub = 33000;//traces[i].max()+10;
            int lb = -33000;//traces[i].min()-10;
            graphs[i].draw_graph(traces[i].get_crop(0,0,traces[i].width()-1,0),red,1,1,1,lb,ub,~0U);
            graphs[i].draw_graph(traces[i].get_crop(0,1,traces[i].width()-1,1),green,1,1,1,lb,ub,~0U);
            graphs[i].draw_graph(traces[i].get_crop(0,2,traces[i].width()-1,2),blue,1,1,1,lb,ub,~0U);
            if (i<=7)
            {
                sensor_img.draw_image(70,(int)(30+20*i+graphs[i].height()*i),graphs[i]);
                sensor_img.draw_text(45,(int)(30+20*i+graphs[i].height()*(i+1)-15),lexical_cast<string>(lb).c_str(),white);
                sensor_img.draw_text(45,(int)(30+20*i+graphs[i].height()*i),lexical_cast<string>(ub).c_str(),white);
            }
            else
            {
                sensor_img.draw_image(1000,(int)(30+20*(i-8)+graphs[i].height()*(i-8)),graphs[i]);
                sensor_img.draw_text(1000-35,(int)(30+20*(i-8)+graphs[i].height()*((i-8)+1)-15),lexical_cast<string>(lb).c_str(),white);
                sensor_img.draw_text(1000-35,(int)(30+20*(i-8)+graphs[i].height()*(i-8)),lexical_cast<string>(ub).c_str(),white);
            }
        }
        sensor_img.display(sensor_disp);
    }
    m_display_running = false;
}
