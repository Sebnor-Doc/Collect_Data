#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

// Thread
#include <QMutex>
#include <QThread>

// General
#include <QImage>
#include <typedef.h>
#include <QTime>

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

    VideoMode mode;
    QTime time;

    // Output Files
    QString subFilePath;
    QString origSubFilePath;

    // Frame characteristics
    int frame_width;
    int frame_height;
    int frame_rate;
    int fourCC;

    // Replay
    VideoCapture replayVideo;
    QString replayFilePath;
    QList<QPixmap> playbackList;
    int playbackIdx;

    // Thread
    QMutex mutex;

public slots:
    void process();
    void setCameraInfo(int frame_width, int frame_height, int fourCC, int fps);
    void setSubFilename(QString filename);
    void setReplay(QString videoPath);
    void saveVideo(bool saveVal);
    void displayVideo(VideoMode mode);
    void updatePlaybackIdx(int);
    void stop();

private slots:
    void emitVideo(Mat* frame);
    void saveFrame(Mat* frame);

signals:
    void processedImage(const QPixmap &image);
    void lipPosition(QVector<QPoint> lipsPos);
    void replayFrameRange(int lower, int upper);
    void stopped();
    void finished();

private:
    void trackLips(Mat &frame);
    Mat extractLipsAsBWImg(Mat &frame);
    QVector<QPoint> extractPointsOnLipsEdge(Mat &binaryImg);

public:
    ~VideoThread();
};

#endif // VIDEOVideoThread_H
