#ifndef MOJOSERIALPORT_H
#define MOJOSERIALPORT_H

#include <QObject>
#include <boost/regex.h>
#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include "common.h"


class MOJOSerialPort: public QObject
{
    Q_OBJECT

public:
    MOJOSerialPort(std::string portName);
    ~MOJOSerialPort();

    void getSinglePacket();
    short getSensorData(int pcb, int sensor, int dim);
    char getRaw(int idx);

    void printRaw();

signals:
    void newPacket(MagData *packet);

public:
  boost::asio::io_service io;
  boost::asio::serial_port *serialport;
  MagData rawData;

private:
//  static const int PACKET_HEADER = 0xAA;
//  static const int PACKET_TAIL = 0xBB;
    static const short PACKET_HEADER = 0xAA;
    static const short PACKET_TAIL = 0xBB;
  static const int PACKET_HEADER_LENGTH = 4;
  static const int PACKET_TAIL_LENGTH = 4;

  int numLostPackets;

};

#endif // MOJOSERIALPORT_H
