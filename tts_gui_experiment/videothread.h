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
    QMutex mutex;

public:
    VideoReadWorker();
    ~VideoReadWorker();
    void run();

public slots:
    void stop();

signals:
    void newFrame(Mat* frame);
    void cameraInfo(int frame_width, int frame_height, int fourCC, int fps);
};

class VideoThread: public QObject
{
    Q_OBJECT

private:
    // General
    VideoWriter video;
    VideoReadWorker readThread;
    bool saveFrame;
    bool dispFrame;


    // Video data
    QImage videoImg;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;
    int fourCC;

    // Thread
    QMutex mutex;

public slots:
    void process();
    void setCameraInfo(int frame_width, int frame_height, int fourCC, int fps);
    void setFilename(QString filename);
    void saveVideo(bool save);
    void displayVideo(bool disp);
    void stop();

private slots:
    void emitVideo(Mat* frame);

 signals:
    void processedImage(const QPixmap &image);
    void stopped();
    void finished();

public:
    ~VideoThread();

};

#endif // VIDEOVideoThread_H
