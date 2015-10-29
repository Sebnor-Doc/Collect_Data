#ifndef PostProcessStreams_H
#define PostProcessStreams_H

#include "common.h"
#include "comparetrajectories.h"
#include <QString>
#include <sndfile.hh>
#include <math.h>
#include <QFile>
#include "VideoThread.h"

class PostProcessStreams: public QObject
{Q_OBJECT
public:
    PostProcessStreams();
    ~PostProcessStreams();

public:
    /* Instance Variables */
    SndfileHandle file;
    double buffer_len;
    QString fname;
    QString version;
    double time_buffer;
    QString experimentalFile;
    QString referenceFile;
    //Time stuff
    double key_start, key_threshold, key_end;
    //Sound
    short *sound_buffer;
    double sample_rate;
    int channels;
    int format = file.format();
    int frames = file.frames();
    //sound
    VideoThread *video;
    //Localization
    CompareTrajectories *ct;
    // other
    int numTrials;

    /* Methods */
    void setParameters(QString fname, double key_start, double key_threshold, double key_end, double time_buffer, VideoThread *video);
    void sound_create_file(QString version);
    void sound_read_file(QString version);
    void process();
    void process_sound();
    void process_video();
    void process_raw();
    void setReferenceFile(QString file);
    void setExperimentalFile(QString file);
    void processTrajectories();
    void formatRefFilename();

};

#endif // PostProcessStreams_H
