#ifndef TYPEDEF
#define TYPEDEF

#include <QObject>
#include <QVector>
#include <QMap>


// Magnetic data
struct MagData{
    QVector<short> packet;
    qint64 time;
    quint8 id;
};
const int dontcare = qRegisterMetaType<MagData>("MagData");


// Audio data
typedef QMap<double, double> AudioSample;   // Map is preferred as easy to get QVectors from keys and values
const int dontcare4 = qRegisterMetaType< AudioSample >("AudioSample");


// Localization datapoint
struct LocaData {
    double x, y, z, theta, phi;
    double time;
    quint8 id;
    QString filename;
};

enum VideoMode { LIVE_FEED, REPLAY_SUB, REPLAY_REF, NO_FEED };
const int dontcare5 = qRegisterMetaType<VideoMode>("VideoMode");

#endif // TYPEDEF

