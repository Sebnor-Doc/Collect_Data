#ifndef TYPEDEF
#define TYPEDEF

#include <QObject>
#include <QVector>
#include <QMap>

// FilePaths
struct RefSubFilePaths {
    QString refLoca;
    QString refMag;
    QString refAudio;
    QString refLips;
    QString subLoca;
    QString subMag;
    QString subAudio;
    QString subLips;
    QString utterClass;
    QString utter;
    int trialNb;
};
const int dontcare8 = qRegisterMetaType<RefSubFilePaths>("RefSubFilePaths");


struct Scores {
    double loca     = 0.0;
    double mag      = 0.0;
    double voice    = 0.0;
    double lips     = 0.0;
    double avg      = 0.0;
};
const int dontcare7 = qRegisterMetaType<Scores>("Scores");

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

enum VideoMode { RAW_FEED, LIP_CONTOUR, BW_FEED, REPLAY_SUB, REPLAY_REF, NO_FEED };
const int dontcare5 = qRegisterMetaType<VideoMode>("VideoMode");


// Video data
const int dontcare6 = qRegisterMetaType< QVector<QPoint> >("QVector<QPoint>");


// Data Checker
struct BadTrial {
    QString category;
    QString utter;
    int trial;
};



#endif // TYPEDEF

