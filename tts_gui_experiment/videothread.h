#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include <vector>


using std::vector;
using namespace cv;

class VideoThread: public QThread
{
    Q_OBJECT

private:

    VideoCapture camera;

    // Thread status
    bool stop;
    bool saveVideo;
    bool showVideo;

    // Video data
    QString filename;
    vector<Mat> *tempVid;
    vector<Mat> *finalVid;
    Mat videoMat;
    QImage videoImg;

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

     Mat RGBframe;


 signals:
     //Signal to output frame to be displayed
     void processedImage(const QPixmap &image);

 public slots:
      void processVideo();

 protected:
     void run();
     void msleep(long ms);
     void usleep(long ms);

 public:
    VideoThread(QObject *parent = 0);
    ~VideoThread();

    void Play();
    void Stop();
    bool isStopped() const;

    void setCamera();
    void setVideoName(QString filename);

    void startSavingVideo();
    void stopSavingVideo();

    void beginEmittingVideo();
    void endEmittingVideo();

    void postProcessVideo();
    void postProcessVideo(int threshold);
    vector<double> linspace(double a, double b, int n); // Implement a linspace function for video normalization

};
#endif // VIDEOVideoThread_H
