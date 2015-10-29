#include "mojoserialport.h"
#include <QDebug>
#include "common.h"


MOJOSerialPort::MOJOSerialPort(std::string port)
{
    serialport = new boost::asio::serial_port(io);
    serialport->open(port);
    serialport->set_option(boost::asio::serial_port_base::baud_rate(500000));
}

MOJOSerialPort::~MOJOSerialPort(){
    serialport->close();
    delete serialport;
}


/* Get data from the Mojo */
void MOJOSerialPort::getSinglePacket() {
    run();
}

void MOJOSerialPort::run(){

    //Purge buffers
    PurgeComm(serialport->native_handle(), PURGE_RXCLEAR);
    numLostPackets = 0;

    char streamchar;
    int cur_num = 0;
    int header[4];
    int tail[4];
    int header_count = 0;
    int tail_count = 0;
    int prev_packetnumber = 0;
    bool header_found = false;
    bool tail_found = false;
    bool gotValid = false;
    std::vector<int> pktData;

    //Clear existing
    rawData.clear();

    //Buffer packet data
    while (!gotValid) {

        while(true) {
            boost::asio::read(*serialport, boost::asio::buffer(&streamchar, 1));

            cur_num = streamchar & 0xFF;

            header[header_count] = cur_num;
            header_count++;

            if(header[0] == PACKET_HEADER && header[1] == PACKET_HEADER && header[2] == PACKET_HEADER && header[3] == PACKET_HEADER )
            {
                header_count = 0;
                header_found = true;
                tail_found = false;
                header[0] = PACKET_TAIL;
                header[1] = PACKET_TAIL;
                header[2] = PACKET_TAIL;
                header[3] = PACKET_TAIL;

            }

            if(header_count > PACKET_HEADER_LENGTH -1 )
            {
                header_count=0;
            }

            if(header_found)
            {
                if(cur_num == PACKET_TAIL)
                {
                    tail[tail_count] = cur_num;
                    tail_count++;

                    if(tail[0] == PACKET_TAIL && tail[1] == PACKET_TAIL && tail[2] == PACKET_TAIL && tail[3] == PACKET_TAIL )
                    {
                        tail_count =0;
                        tail_found = true;
                        header_found = false;
                        tail[0] = PACKET_HEADER;
                        tail[1] = PACKET_HEADER;
                        tail[2] = PACKET_HEADER;
                        tail[3] = PACKET_HEADER;
                        pktData.erase(pktData.begin());
                        pktData.pop_back();
                        pktData.pop_back();
                        pktData.pop_back();
                        break;
                    }

               }

                if(tail_count > PACKET_TAIL_LENGTH -1 )
                {
                    tail_count=0;
                }

                pktData.push_back(cur_num);

            }

        }

        //Check packet size constraint
        if (pktData.size() == EXPECTEDBYTES) {
            gotValid = true;

            if(pktData[0]!= prev_packetnumber)
            {
                numLostPackets++;
//                qDebug() << "packet " << pktData[0] <<  "  is missing." << endl;

            }

            //Handle data
            for (int i = NOOFBYTESINPC; i < pktData.size(); i++) {
                rawData.push_back(pktData[i]);
            }
            emit newPacketAvail(rawData);

            prev_packetnumber = pktData[0];
            prev_packetnumber++;
        }

        pktData.clear();
    }
}


/* Read a value from rawData */
short MOJOSerialPort::getSensorData(int pcb, int sensor, int dim){

    int datumIdx = pcb * NUM_OF_SENSORS_PER_BOARD * NUM_OF_BYTES_PER_SENSOR + sensor * NUM_OF_BYTES_PER_SENSOR + dim * 2;

    char lower = rawData[datumIdx] & 0xFF;
    char upper = rawData[datumIdx + 1] & 0xFF;

    return ((upper << 8) | (lower & 0xFF));
}

char MOJOSerialPort::getRaw(int Idx){
    return rawData[Idx] & 0xFF;
}

/* Display rawData values */
void MOJOSerialPort::printRaw(){
    for (int i = 0; i < rawData.size(); i++) {
        printf("%x ", rawData[i] & 0xFF );
    }
    printf("\n");
}

