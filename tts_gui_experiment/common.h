#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QVector>

// System characteristics
const int BAUD_RATE = 500000;
const int NUM_OF_SENSORS_PER_BOARD = 4;
const int NUM_OF_BOARDS = 6;
const int NUM_OF_SENSORS = NUM_OF_SENSORS_PER_BOARD * NUM_OF_BOARDS;
const int NUM_OF_BYTES_PER_SENSOR = 6;
const int NOOFBYTESINPC = 1;
const int EXPECTEDBYTES = 6 * NUM_OF_SENSORS + NOOFBYTESINPC;


// Typedef for Magnetic data
struct MagData{
    QVector<short> packet;
    qint64 time;
    quint8 id;
};
const int dontcare = qRegisterMetaType<MagData>("MagData"); // Needed for Signal/Slot connections

// Typedef for Audio data
typedef QMap<double, double> AudioSample;   // Map is preferred as easy to get QVectors from keys and values
const int dontcare4 = qRegisterMetaType< AudioSample >("AudioSample");

//Plot details
const int RANGE_VALS = 33000;

// Others
const int NUM_PTS = 100;
const int AVG_N = 10;

#endif // COMMON_H
