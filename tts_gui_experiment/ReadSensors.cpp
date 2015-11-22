#include "ReadSensors.h"
#include <time.h>
#include <QUrl>
#include <QDebug>
#include <QDateTime>

#include <QMessageBox>

#include <QSerialPortInfo>
#include <QApplication>

ReadSensors::ReadSensors(QObject *parent): QThread(parent)
{
    // Set thread state
    stop = false;
    save = false;

    // Connect to Mojo by identifying its COM Port
    QSerialPortInfo mojoComPort;
    QList<QSerialPortInfo> listOfPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, listOfPorts) {
        if (port.productIdentifier() == 32769) {
            mojoComPort = port;
            break;
        }
    }

    // Set Mojo Serial Port
    if (!mojoComPort.portName().isEmpty()) {

        serialport = new boost::asio::serial_port(io);

        try {
            serialport->open(mojoComPort.portName().toStdString());
            qDebug() << "Mojo COM Port = " << mojoComPort.portName() << "\n";
        }
        catch(...) {
            qDebug() << "\nERROR: CANNOT OPEN MOJO AT - " << mojoComPort.portName() << "\n";
        }

        serialport->set_option(boost::asio::serial_port_base::baud_rate(BAUD_RATE));
    }
    else {
        qDebug() << "ERROR: NO COM PORT ASSOCIATED TO MOJO IS FOUND!!";
        QMessageBox::critical((QWidget*)parent, "Mojo Not Found", "ERROR: NO COM PORT ASSOCIATED TO MOJO IS FOUND!!", QMessageBox::Ok, QMessageBox::NoButton);
    }

    // manage connection
    connect(this, SIGNAL(packetRead(MagData)), this, SLOT(processPacket(MagData)));
}

void ReadSensors::run()
{
    numLostPackets = 0;

    while (!stop) {
        readPacket();
    }

}

/* Manage Packet */
void ReadSensors::readPacket() {

    //Purge buffers
    PurgeComm(serialport->native_handle(), PURGE_RXCLEAR);

    // Initialize variables
    char streamchar;
    short cur_num;
    int prev_packetnumber = 0;
    bool packetFound = false;

    // Get a packet
    while (!packetFound) {

        int header_count = 0;
        int tail_count = 0;
        bool header_found = false;
        bool tail_found = false;
        std::vector<short> pktData;

        // Identify the header from the data stream
        while(!tail_found) {
            // Read a byte from serial port
            boost::asio::read(*serialport, boost::asio::buffer(&streamchar, 1));
            cur_num = streamchar & 0xFF;

            if (!header_found) {

                if (cur_num == PACKET_HEADER) {
                    header_count++;
                    header_found = (header_count == PACKET_HEADER_LENGTH);
                }
                else {
                    header_count = 0;
                }
            }

            else if (!tail_found) {
                 pktData.push_back(cur_num);

                 if (cur_num == PACKET_TAIL) {
                     tail_count++;
                     tail_found = (tail_count == PACKET_TAIL_LENGTH);

                     if (tail_found) {
                         pktData.erase( pktData.end() - PACKET_TAIL_LENGTH, pktData.end());
                     }

                 }
                 else {
                     tail_count = 0;
                 }

            }
        }

        packetFound = (pktData.size() == EXPECTEDBYTES);

        if (packetFound) {

            if(pktData[0]!= prev_packetnumber)
            {
                numLostPackets++;
            }

            MagData tempPacket;

            // Format packet data by combining 2 bytes per magnetic axis
            for (int i = NOOFBYTESINPC; i < EXPECTEDBYTES - 1; i += 2) {

                char lower = pktData[i] & 0xFF;
                char upper = pktData[i + 1] & 0xFF;
                short data = ((upper << 8) | (lower & 0xFF));

                tempPacket.push_back(data);
            }

            prev_packetnumber = pktData[0];
            prev_packetnumber++;

            mutex.lock();
            magPacket = tempPacket;
            mutex.unlock();

            emit packetRead(magPacket);
        }
    }
}

void ReadSensors::processPacket(MagData packet) {
    mutex.lock();

    // Save raw magnetic information
    if (save)
    {
        for (int i = 0; i < packet.size(); i++) {
            (*sensorOutputFile_stream) << packet.at(i) << " ";
        }

        (*sensorOutputFile_stream) << QDateTime::currentDateTime().toMSecsSinceEpoch() << endl;

        emit packetSaved(packet);
    }

    mutex.unlock();
}

MagData ReadSensors::getLastPacket()
{
    return this->magPacket;
}


/* Manage REcording status */
void ReadSensors::stopRecording()
{
    stop = true;
}

void ReadSensors::beginRecording()
{
    stop = false;
}


/* Manage Saving */
void ReadSensors::setFileLocation(QString filename)
{
    mutex.lock();
    this->filename = filename;
    mutex.unlock();
}

void ReadSensors::startSaving()
{
    mutex.lock();
    save = true;
    sensorOutputFile.setFileName(filename);
    sensorOutputFile_stream = new QTextStream(&sensorOutputFile);
    sensorOutputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    mutex.unlock();
}

void ReadSensors::stopSaving()
{
    mutex.lock();
    save = false;
    sensorOutputFile.close();
    mutex.unlock();
}

/* Other */
ReadSensors::~ReadSensors()
{
    serialport->close();
}
