#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QVector>
#include <typedef.h>

// System characteristics
const int BAUD_RATE = 500000;
const int NUM_OF_SENSORS_PER_BOARD = 4;
const int NUM_OF_BOARDS = 6;
const int NUM_OF_SENSORS = NUM_OF_SENSORS_PER_BOARD * NUM_OF_BOARDS;
const int NUM_OF_BYTES_PER_SENSOR = 6;
const int NOOFBYTESINPC = 1;
const int EXPECTEDBYTES = 6 * NUM_OF_SENSORS + NOOFBYTESINPC;

//Plot details
const int RANGE_VALS = 33000;

// Others
const int NUM_PTS = 100;
const int AVG_N = 10;

#endif // COMMON_H
