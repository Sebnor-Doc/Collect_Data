#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

// Thread
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

// OpenCV
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;

// Other
#include <QImage>


class VideoThread: public QThread
{
    Q_OBJECT

private:
    // General
    VideoCapture camera;
    VideoWriter *video;
    bool saveVideo;
    bool showVideo;

    // Video data
    QString filename;
    Mat videoMat;
    QImage videoImg;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;

    // Thread
    bool stop;
    QMutex mutex;
    QWaitCondition condition;


 public:
    VideoThread(QObject *parent = 0);
    ~VideoThread();

    void Play();
    void Stop();

    void setVideoName(QString filename);

    void startSavingVideo();
    void stopSavingVideo();

    void startEmittingVideo();
    void stopEmittingVideo();


 signals:
    void processedImage(const QPixmap &image);


 private:
    void setCamera();
    void processVideo();
    bool isStopped() const;


 protected:
    void run();

};
#endif // VIDEOVideoThread_H
