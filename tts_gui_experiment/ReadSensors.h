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


class ReadSensors: public QThread
{
    Q_OBJECT

private:
    boost::asio::io_service io;
    boost::asio::serial_port *serialport;
    MagData magPacket;

    // Thread status
    bool stop;
    bool save;
    QMutex mutex;

    // Saving
    QString filename;
    QFile sensorOutputFile;
    QTextStream *sensorOutputFile_stream;

    // Mojo parameters
    static const short PACKET_HEADER = 0xAA;
    static const short PACKET_TAIL = 0xBB;
    static const int PACKET_HEADER_LENGTH = 4;
    static const int PACKET_TAIL_LENGTH = 4;
    int numLostPackets;

signals:
    void newPacketAvail(MagData packet);

private slots:
    void processPacket(MagData packet);

public:
    ReadSensors(QObject *parent = 0);
    ~ReadSensors();
    void run();
    void stopRecording();
    void beginRecording();
    MagData getLastPacket();
    void setFileLocation(QString filename);
    void startSaving();
    void stopSaving();

private:
    void readPacket();
};
#endif // ReadSensors_H
