#include "roboticarmwrapper.h"
#include "TimeoutSerial.h"

TimeoutSerial *serial;
using namespace boost;

roboticArmWrapper::roboticArmWrapper(ReadSensors *rs, QString COM)
{
    this->rs = rs;

    posX = 0;
    posY = 0;
    posZ = 0;

    if (ROBOTIC_ARM_CONNECTED)
    {
        asio::serial_port_base::parity *paritySetting = new asio::serial_port_base::parity(asio::serial_port_base::parity::type::none);
        asio::serial_port_base::character_size *characterSize = new asio::serial_port_base::character_size();
        asio::serial_port_base::flow_control *flow = new asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::hardware);
        asio::serial_port_base::stop_bits *stop = new asio::serial_port_base::stop_bits();
        serial = new TimeoutSerial(COM.toStdString().c_str(), 115200, *paritySetting, *characterSize, *flow, *stop);
        serial->setTimeout(posix_time::seconds(1));
    }
}


bool roboticArmWrapper::parseJSON(QString t) // not a true parser
{
    bool out = false;

    int findPos, start, stop1, stop2, stop;

    //Find current X position
    findPos = t.indexOf("\"posx\":");
    findPos = (findPos == -1)?t.indexOf("\"mpox\":"):findPos;
    if (findPos != -1)
    {
        start = findPos + 7;
        stop1 = t.indexOf(",", start);
        stop2 = t.indexOf("}", start);

        stop1 = (stop1 == -1)?t.length():stop1;
        stop2 = (stop2 == -1)?t.length():stop2;

        stop = stop1<stop2?stop1:stop2;

        posX = t.mid(start, stop-start).toDouble();
        // Convert to SI units
        posX /= 1000;
        out = true;
    }

    //Find current Y position
    findPos = t.indexOf("\"posy\":");
    findPos = (findPos == -1)?t.indexOf("\"mpoy\":"):findPos;
    if (findPos != -1)
    {
        start = findPos + 7;
        stop1 = t.indexOf(",", start);
        stop2 = t.indexOf("}", start);

        stop1 = (stop1 == -1)?t.length():stop1;
        stop2 = (stop2 == -1)?t.length():stop2;

        stop = stop1<stop2?stop1:stop2;

        posY = t.mid(start, stop-start).toDouble();
        // Convert to SI units
        posY /= 1000;
        out = true;
    }

    //Find current Z position
    findPos = t.indexOf("\"posz\":");
    findPos = (findPos == -1)?t.indexOf("\"mpoz\":"):findPos;
    if (findPos != -1)
    {
        start = findPos + 7;
        stop1 = t.indexOf(",", start);
        stop2 = t.indexOf("}", start);

        stop1 = (stop1 == -1)?t.length():stop1;
        stop2 = (stop2 == -1)?t.length():stop2;

        stop = stop1<stop2?stop1:stop2;

        posZ = t.mid(start, stop-start).toDouble();
        // Convert to SI units
        posZ /= 1000;
        out = true;
    }
    //qDebug()<<posX<<","<<posY<<","<<posZ<<endl;

    return out;
}

void roboticArmWrapper::moveTrajectory(QString traj_file)
{
    //Write to this file
    this->traj_file = traj_file;
}

void roboticArmWrapper::process()
{
    QFile rW_pos_raw(filename);
    rw_pos_raw_stream = new QTextStream(&rW_pos_raw);
    rW_pos_raw.open(QIODevice::WriteOnly | QIODevice::Text);

    //Read from this file
    QFile traj_input(traj_file);
    QTextStream traj_input_stream(&traj_input);

    if (traj_input.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QStringList robArmPos;
        double pos[3];

        while (!traj_input_stream.atEnd())
        {
            /* POSITION DATA*/
            robArmPos = traj_input_stream.readLine().split(" ");
            pos[0] = robArmPos[0].toDouble();
            pos[1] = robArmPos[1].toDouble();
            pos[2] = robArmPos[2].toDouble();

            // Wait till robotic arm reaches this point
            // Will save to file as well - bad practice, but oh well
            moveTo(pos[0],pos[1],pos[2]);
        }
        rW_pos_raw.close();
        traj_input.close();
    }

    emit finished();

}


void roboticArmWrapper::moveTo(double x, double y, double z)
{

    if (ROBOTIC_ARM_CONNECTED)
    {
        // Convert m to mm
        x *= 1000;
        y *= 1000;
        z *= 1000;
        std::string command = (QString("g0 x" + QString::number(x) + " y" + QString::number(y) +" z" + QString::number(z))+"\n").toStdString();
        serial->writeString(command);
        saveToFile();
        //Keep flow of control here till etc

    }
}

void roboticArmWrapper::saveTo(QString filename)
{
    this->filename = filename;
}

void roboticArmWrapper::saveToFile()
{
    if (ROBOTIC_ARM_CONNECTED)
    {
        try
        {
            while (true) // yes, this is bad practice
            {
                QString rWOutput = QString::fromUtf8(serial->readStringUntil("\n").c_str());

                //qDebug()<<rWOutput;

                if (parseJSON(rWOutput))
                {
                    (*rw_pos_raw_stream)<<posX<<" "<<posY<<" "<<posZ<<" ";

                    for (int i=0; i<NUM_OF_SENSORS; i++)
                    {
                        (*rw_pos_raw_stream) <<rs->getSensorData(i/4, i%4, 0)<<" "
                                             <<rs->getSensorData(i/4, i%4, 1)<<" "
                                             <<rs->getSensorData(i/4, i%4, 2)<<" ";
                    }
                    (*rw_pos_raw_stream)<<endl;
                }
            }

        }
        catch(...)
        {

        }
    }
}
