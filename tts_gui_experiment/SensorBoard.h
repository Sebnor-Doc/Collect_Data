/*
 * SensorBoard.h
 *
 *  Created on: Apr 26, 2012
 *      Author: jacob
 */

#ifndef SENSORBOARD_H_
#define SENSORBOARD_H_

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/timer/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <deque>
#include <vector>
#include <fstream>

#include "TimeoutSerial.h"
#include "Measurement.h"
#include "CImg.h"
#include "constants.h"


class SensorBoard: protected TimeoutSerial {


private:
    std::string m_name, m_fileOutputName;
    unsigned int m_channel, m_timeout;
    std::deque< Measurement > m_data;
    std::ofstream m_fileOutput;
    boost::thread m_threadRead;
    boost::mutex m_mutexRead;
    double m_battery;
    bool handshake();
    void readBoard();
    boost::thread m_display_thread;
    boost::mutex m_display_mutex;
    void m_display();
    bool m_display_running;
public:
    SensorBoard(const std::string& devname, const unsigned int channel, const bool handshake, const std::string fileOutputName, const unsigned int timeout = 2);
    SensorBoard(int board_number, const std::string fileOutputName);
    virtual ~SensorBoard();
    std::deque< Measurement > data(int size = 1);
    std::string name();
    double battery();
    void display();
};

#endif /* SENSORBOARD_H_ */
