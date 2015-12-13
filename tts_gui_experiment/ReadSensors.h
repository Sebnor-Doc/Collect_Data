#ifndef ReadSensors_H
#define ReadSensors_H

#include "common.h"
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QThread>

#include <boost/regex.h>
#include <boost/asio/serial_port.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>


class MagReadWorker: public QThread
{
    Q_OBJECT

public:
    MagReadWorker();
    void run();
    void setSerialPort(boost::asio::serial_port *sp);

public slots:
    void stop();

private:
    void readPacket();

signals:
    void packetRead(MagData packet);

private:
    boost::asio::serial_port *sp;
    bool stopExec;
    QMutex mutex;

    // Mojo parameters
    static const short PACKET_HEADER        = 0xAA;
    static const short PACKET_TAIL          = 0xBB;
    static const int PACKET_HEADER_LENGTH   = 4;
    static const int PACKET_TAIL_LENGTH     = 4;
};

class ReadSensors: public QObject
{
    Q_OBJECT

private:
    MagReadWorker readMagSens;
    MagData magPacket;
    qint64 baseTime;

    // Serial Port
    boost::asio::io_service io;
    boost::asio::serial_port *serialport;

    // Saving
    QFile sensorOutputFile;
    QTextStream outputFileStream;

    // Others
    int numLostPackets;
    QMutex mutex;

signals:
    void stopReading();
    void lastPacket(MagData);
    void finished();

public slots:
    void process();
    void setSubFilename(QString subFileRoot);
    void saveMag(bool save);
    void getLastPacket();
    void stop();

private slots:
    void updateLastPacket(MagData magData);
    void savingMag(MagData magData);

public:
    ~ReadSensors();
    void setFilename(QString filename);

};
#endif // ReadSensors_H
