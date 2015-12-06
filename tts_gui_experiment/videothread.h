#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

// Thread
#include <QMutex>
#include <QThread>

// General
#include <QImage>

// OpenCV
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
using namespace cv;

// Needed to use Mat in signal/slot connections
Q_DECLARE_METATYPE(Mat*)
const int dontcare2 = qRegisterMetaType<Mat*>();


class VideoReadWorker: public QThread {
    Q_OBJECT

private:
    VideoCapture camera;
    bool stopExec;
    Mat frame;

public:
    VideoReadWorker();
    ~VideoReadWorker();
    void run();

public slots:
    void stop();

signals:
    void newFrame(Mat* frame);
    void cameraInfo(int frame_width, int frame_height);
};

class VideoThread: public QObject
{
    Q_OBJECT

private:
    // General
    VideoWriter video;
    VideoReadWorker readThread;

    // Video data
    QImage videoImg;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;

    // Thread
    QMutex mutex;

public slots:
    void process();
    void setCameraInfo(int frame_width, int frame_height);
    void setFilename(QString filename);
    void saveVideo(bool save);
    void displayVideo(bool disp);   
    void stop();

private slots:
    void savingVideo(Mat* frame);
    void emitVideo(Mat* frame);

 signals:
    void processedImage(const QPixmap &image);
    void finished();

};

#endif // VIDEOVideoThread_H
