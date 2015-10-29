#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <QTimer>
#include <QtMultimedia/QAudioRecorder>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <vector>


using std::vector;
using namespace cv;

class VideoThread: public QThread
{    Q_OBJECT

 private:
    // Thread status
    bool stop;
    bool saveVideo;
    bool showVideo;

    // Video data
    QString filename;
    vector<Mat> *tempVid;
    vector<Mat> *finalVid;
    Mat matOriginal;
    QImage img;

    // Call
    VideoWriter *video;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;

    // Time values
    double start_time, end_time;
    long sleepTime;

    // To be sorted
    int i;
     QMutex mutex;
     QWaitCondition condition;
     Mat frame;
     int frameRate;
     VideoCapture capture;
     Mat RGBframe;


 signals:
     //Signal to output frame to be displayed
     void processedImage(const QImage &image);

 public slots:
      void emitVideo();

 protected:
     void run();
     void msleep(long ms);
     void usleep(long ms);

 public:
    //Constructor
    VideoThread(QObject *parent = 0);
    //Destructor
    ~VideoThread();
    //Run the webcam
    void Play();
    //Stop
    void Stop();
    //check if the VideoThread has been stopped
    bool isStopped() const;
    // Start Webcam
    void startWebcam();
     // End Webcam
     void endWebcam();
    //Set Video Name
    void setVideoName(QString filename);
    //Start saving video
    void startVideo();
    //End saving video
    void endVideo();
    //Begin emitting video to GUI
    void beginEmittingVideo();
    //End emitting video to GUI
    void endEmittingVideo();
    //Implement a linspace function for video normalization
    vector<double> linspace(double a, double b, int n);
    //postProcessVideo
    void postProcessVideo();
    //postProcessVideo with only lip motion
    void postProcessVideo(int threshold);


};
#endif // VIDEOVideoThread_H
