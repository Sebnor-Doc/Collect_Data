#include "PostProcessStreams.h"

PostProcessStreams::PostProcessStreams()
{
    ct = new CompareTrajectories();
}

PostProcessStreams::~PostProcessStreams()
{

}

void PostProcessStreams::setParameters(QString fname, double key_start, double key_threshold, double key_end, double time_buffer, VideoThread *video)
{
    this->fname = fname;
    this->key_start = key_start;
    this->key_threshold = key_threshold;
    this->key_end = key_end;
    this->time_buffer = time_buffer;
    this->video = video;
}

void PostProcessStreams::sound_create_file(QString version)
{
    SndfileHandle wfile;

    QByteArray temp = (fname+version+"_post.wav").toUtf8();
    wfile = SndfileHandle (temp.constData(), SFM_WRITE, format, channels, sample_rate);

    int start_at = (int) round((key_threshold-key_start - time_buffer)*sample_rate/1000.0);
    wfile.write(&sound_buffer[start_at], frames-start_at);
}

void PostProcessStreams::sound_read_file(QString version)
{
    QByteArray temp = (fname+version+".wav").toUtf8();

    file = SndfileHandle (temp.constData()) ;

    buffer_len = file.frames();
    sample_rate = file.samplerate();
    channels =  file.channels ();
    format = file.format();
    frames = file.frames();

    sound_buffer = new short[frames];
    file.read (sound_buffer, frames) ;
}

void PostProcessStreams::process()
{
    //Time Synchronization
    process_sound();
    process_raw();
    process_video();

}

void PostProcessStreams::process_sound()
{
    sound_read_file("_audio1");
    sound_create_file("_audio1");

    sound_read_file("_audio2");
    sound_create_file("_audio2");
}

void PostProcessStreams::process_video()
{
    video->postProcessVideo(key_threshold-key_start - time_buffer);
}

void PostProcessStreams::process_raw()
{
    QFile input(fname+"_raw_sensor.txt");
    QTextStream in(&input);
    QFile output(fname+"_raw_sensor_post.txt");
    QTextStream out(&output);

    input.open(QIODevice::ReadOnly | QIODevice::Text);
    output.open(QIODevice::WriteOnly | QIODevice::Text);

    QStringList reading;

    while(!in.atEnd())
    {
        reading = in.readLine().split(" ");

        if (reading[reading.length()-1].toDouble() >= key_threshold - time_buffer)
        {
            for (int i=0; i<reading.length()-1; i++)
            {
                out<< reading[i] << " ";
            }
            out << reading[reading.length()-1] << endl;
        }
    }
    input.close();
    output.close();
}

void PostProcessStreams::setReferenceFile(QString file)
{
    referenceFile = file;
}

void PostProcessStreams::setExperimentalFile(QString file)
{
    experimentalFile = file;
}

void PostProcessStreams::processTrajectories()
{
    formatRefFilename();
    ct->loadReferenceFile(referenceFile);
    ct->loadExperimentalFile(experimentalFile);

    //Processes trajectory/sets up plot
    ct->processTrajectory();

    //Emit trajectory difference for realtime plot
    ct->emitTrajectoryDifference();
}

void PostProcessStreams::formatRefFilename()
{
    for (int i = 2; i <= numTrials; i++) {
        referenceFile.replace(QString::number(i),"1");
    }
}


