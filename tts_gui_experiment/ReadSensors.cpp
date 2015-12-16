#include "ReadSensors.h"
#include <time.h>
#include <QUrl>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QApplication>

/* ************************
 * ReadSensors Class
 * ************************ */

void ReadSensors::process(){

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
        QMessageBox msgBox;
        msgBox.setText("ERROR: NO COM PORT ASSOCIATED TO MOJO IS FOUND!!");
        msgBox.exec();
    }

    // Create Mag sensor read thread
    readMagSens.setSerialPort(serialport);
    readMagSens.start(QThread::HighestPriority);
    connect(this, SIGNAL(stopReading()), &readMagSens, SLOT(stop()), Qt::DirectConnection);

    // Update internal Mag data variable for last packet
    connect(&readMagSens, SIGNAL(packetRead(MagData)), this, SLOT(updateLastPacket(MagData)));

}

void ReadSensors::saveMag(bool save)
{
    mutex.lock();
    if (save) {
        outputFileStream.setDevice(&sensorOutputFile);
        sensorOutputFile.open(QIODevice::WriteOnly | QIODevice::Text);
        baseTime = -1;
        connect(&readMagSens, SIGNAL(packetRead(MagData)), this, SLOT(savingMag(MagData)));
    }
    else {
        disconnect(&readMagSens, SIGNAL(packetRead(MagData)), this, SLOT(savingMag(MagData)));

        if (sensorOutputFile.isOpen()) {
            sensorOutputFile.close();
        }
    }
    mutex.unlock();
}

void ReadSensors::savingMag(MagData magData) {
    mutex.lock();

    if(baseTime == -1) {
        baseTime = magData.time;
    }

    // Save raw magnetic information
    for (int i = 0; i < magData.packet.size(); i++) {
        outputFileStream << magData.packet.at(i) << " ";
    }

    // Save formatted time
    qint64 relativeTime = magData.time - baseTime;
    outputFileStream << relativeTime << " " << magData.id << endl;
    magData.time = relativeTime;

    // Signal Localization manager to process this mag. packet only if not in EMF saving mode
    if (!emf) {
        emit dataToLocalize(magData);
    }

    mutex.unlock();
}

void ReadSensors::updateLastPacket(MagData magData)
{
    mutex.lock();
    magPacket = magData;
    mutex.unlock();
}

void ReadSensors::getLastPacket()
{
    mutex.lock();
    MagData currentPacket = magPacket;
    mutex.unlock();

    emit lastPacket(currentPacket);
}

void ReadSensors::setFilename(QString filename)
{
    mutex.lock();
    if (sensorOutputFile.isOpen()) {
        sensorOutputFile.close();
    }
    sensorOutputFile.setFileName(filename);
    mutex.unlock();
}

void ReadSensors::setEmf(bool emf)
{
    mutex.lock();
    this->emf = emf;
    mutex.unlock();
}

void ReadSensors::setSubFilename(QString subFileRoot) {
    setFilename(subFileRoot +"_raw_sensor.txt" );
}

void ReadSensors::stop() {

    saveMag(false);
    emit stopReading();
}

ReadSensors::~ReadSensors(){
    emit finished();
}


/* ************************
 * MagReadWorker Class
 * ************************ */

MagReadWorker::MagReadWorker(): QThread() {
    stopExec = false;
}

void MagReadWorker::setSerialPort(boost::asio::serial_port *sp){
    mutex.lock();
    this->sp = sp;
    mutex.unlock();
}

void MagReadWorker::run(){

    //Purge buffers
    PurgeComm(sp->native_handle(), PURGE_RXCLEAR);

    while(!stopExec) {
        readPacket();
    }

    // Closing procedures
    sp->close();
}

void MagReadWorker::readPacket(){

    // Initialize variables
    char streamchar;
    short cur_num;
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
            boost::asio::read(*sp, boost::asio::buffer(&streamchar, 1));
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
            // Create new instance of a packet
            MagData magPacket;
            magPacket.time = QDateTime::currentMSecsSinceEpoch();
            magPacket.id = pktData[0];

            // Format packet data by combining 2 bytes per magnetic axis
            for (int i = NOOFBYTESINPC; i < EXPECTEDBYTES - 1; i += 2) {

                char lower = pktData[i] & 0xFF;
                char upper = pktData[i + 1] & 0xFF;
                short data = ((upper << 8) | (lower & 0xFF));

                magPacket.packet.push_back(data);
            }

            // Emit a signal with final read packet
            emit packetRead(magPacket);
        }
    }

}

void MagReadWorker::stop(){
    mutex.lock();
    stopExec = true;
    mutex.unlock();
}
