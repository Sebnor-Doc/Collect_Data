#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

// Thread
#include <QMutex>

// OpenCV
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;

// Other
#include <QImage>


class VideoThread: public QObject
{
    Q_OBJECT

private:
    // General
    VideoCapture camera;
    VideoWriter video;

    // Video data
    Mat videoMat;
    QImage videoImg;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;

    // Thread
    bool stopExec;
    QMutex mutex;


public:
    VideoThread();
    ~VideoThread();

public slots:
    void process();
    void saveVideo(bool save);
    void displayVideo(bool disp);
    void setFilename(QString filename);
    void stop();

private slots:
    void savingVideo(Mat& frame);
    void emitVideo(Mat& frame);

 signals:
    void newFrame(Mat& frame);
    void processedImage(const QPixmap &image);
    void finished();

 private:
    bool setCamera();

};
#endif // VIDEOVideoThread_H
