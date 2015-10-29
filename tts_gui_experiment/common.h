#ifndef COMMON_H
#define COMMON_H

#include <QDebug>

//System characteristics
const int NUM_OF_SENSORS_PER_BOARD = 4;
const int NUM_OF_BOARDS = 6;
const int NUM_OF_SENSORS = NUM_OF_SENSORS_PER_BOARD * NUM_OF_BOARDS;

const int NOOFBYTESINPC = 1;
const int EXPECTEDBYTES = 6*NUM_OF_SENSORS + NOOFBYTESINPC;
const int NUM_OF_BYTES_PER_SENSOR = 6;

//const int EXPECTEDBYTES = 122;
//const int PACKET_HEADER = 0xAA;
//const int PACKET_SIZE = 145;


//Plot details
const int NUM_ROWS_PLOTS = 6;
const int NUM_COLS_PLOTS = 4;
const int RANGE_VALS = 33000;
const int SENSOR_WORKING_THRESHOLD = 100;
const int LPlot_count = 1;


// Others
const double PI = 3.14159265358979323846;
const int TIME_BUFFER = 200; //Introduce a delay before recording (in ms)
const int NUM_PTS = 100;
const int AVG_N = 10;
//const QString emfFileLoc = "C:/TTS_GUI/tts_gui_experiment/EMF.txt";
#endif // COMMON_H
