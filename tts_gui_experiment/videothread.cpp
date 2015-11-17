#include "VideoThread.h"
#include <QDateTime>
#include <time.h>
#include <QUrl>
#include <QCameraInfo>
#include <QPixmap>


VideoThread::VideoThread(QObject *parent)
    : QThread(parent)
{
    stop = true;
    saveVideo = false;
    showVideo = false;
}


// Start video recording
void VideoThread::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(HighestPriority);
    }
}

void VideoThread::run()
{
    setCamera();

    while (!stop)
    {
        mutex.lock();
        processVideo();
        mutex.unlock();
    }
}

void VideoThread::setCamera()
{
    // Find index of LifeCam webcam
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    int cameraIdx = -1;

    for (int i = 0; i < cameras.size(); i++) {
        if (cameras.at(i).description().contains("LifeCam")) {
            cameraIdx = i;
            break;
        }
    }

    // Open LifeCam or throw runtime exception if cannot be found
    if (cameraIdx == -1) {
        QString errorMsg = "LifeCam camera cannot be found";
        qDebug() << errorMsg << endl;

    } else {
        camera.open(cameraIdx);
        frame_width  = camera.get(CV_CAP_PROP_FRAME_WIDTH);
        frame_height = camera.get(CV_CAP_PROP_FRAME_HEIGHT);
        frame_rate = 30;
        qDebug() << "LifeCam connected successfully";
    }
}

void VideoThread::setVideoName(QString filename)
{
    this->filename = filename;
}

void VideoThread::processVideo()
{
    bool readSuccessful = camera.read(videoMat);
    cv::Mat origVideoMat = videoMat.clone();

    if (!readSuccessful || videoMat.empty()) {          // if we did not get a frame
        qDebug() << "ERROR: CANNOT READ VIDEO IMAGES";
    }
    cv::cvtColor(videoMat, videoMat, CV_BGR2RGB);       // invert BGR to RGB
    videoImg = QImage((uchar*)videoMat.data, videoMat.cols, videoMat.rows, videoMat.step, QImage::Format_RGB888);

    if (showVideo) {
        emit processedImage(QPixmap::fromImage(videoImg));
    }

    if (saveVideo)
    {
        tempVid->push_back(origVideoMat.clone());
    }
}


// Save video
void VideoThread::Stop()
{
    stop = true;
    camera.release();
}

void VideoThread::startSavingVideo()
{
    tempVid = new vector<Mat>();
    mutex.lock();
    saveVideo = true;
    start_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    mutex.unlock();
}

void VideoThread::stopSavingVideo()
{
    mutex.lock();
    saveVideo = false;
    end_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    postProcessVideo();
    mutex.unlock();
}


// Show video
void VideoThread::beginEmittingVideo()
{
    showVideo = true;
}

void VideoThread::endEmittingVideo()
{
    showVideo = false;
    //Clear image
    videoImg = QImage(videoMat.cols, videoMat.rows, QImage::Format_RGB888);
    emit processedImage(QPixmap::fromImage(videoImg));
}


// Processing video data
void VideoThread::postProcessVideo()
{
    finalVid = new vector<Mat>();
    int numFrames = tempVid->size();
    double time_diff = (end_time - start_time)/1000.0;
    double desiredFPS = frame_rate;
    int desiredFrames = desiredFPS*time_diff;

    vector<double> newIndices = linspace(0, numFrames-1, desiredFrames);

    video = new VideoWriter(filename.toStdString(),CV_FOURCC('M','J','P','G'),frame_rate, Size(frame_width,frame_height),true);
    for (int i = 0; i < newIndices.size(); i++)
    {
        finalVid->push_back(tempVid->at((int)round(newIndices.at(i))));
        video->write(finalVid->at(i));
    }
    video->release();
}

vector<double> VideoThread::linspace(double a, double b, int n)
{
    vector<double> array;
    double step = (b-a)/(n-1);

    while(a <= b) {
        array.push_back(a);
        a += step;           // could recode to better handle rounding errors
    }
    return array;
}


// Other
bool VideoThread::isStopped() const{
    return this->stop;
}

// Put thread to sleep
void VideoThread::msleep(long ms){
    QThread::msleep(ms);
}

void VideoThread::usleep(long us){
    QThread::usleep(us);
}


// Destructor
VideoThread::~VideoThread()
{
    mutex.lock();
    stop = true;
    camera.release();
    condition.wakeOne();
    mutex.unlock();
    wait();
}
