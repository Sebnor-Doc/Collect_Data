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


//Plot details
const int RANGE_VALS = 33000;
//const int SENSOR_WORKING_THRESHOLD = 100;
//const int LPlot_count = 1;


// Others
const int NUM_PTS = 100;
const int AVG_N = 10;

#endif // COMMON_H
