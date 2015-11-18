#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>


VideoThread::VideoThread(QObject *parent): QThread(parent)
{
    stop = true;
    saveVideo = false;
    showVideo = false;
}

// Start thread
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


// Display and/or save video data
void VideoThread::processVideo()
{
    bool readSuccessful = camera.read(videoMat);

    if (!readSuccessful || videoMat.empty()) {          // if we did not get a frame
        qDebug() << "ERROR: CANNOT READ VIDEO IMAGES";
        return;
    }

    cv::Mat origVideoMat = videoMat.clone();

    if (showVideo) {
        cv::cvtColor(videoMat, videoMat, CV_BGR2RGB);       // invert BGR to RGB
        videoImg = QImage((uchar*)videoMat.data, videoMat.cols, videoMat.rows, videoMat.step, QImage::Format_RGB888);

        emit processedImage(QPixmap::fromImage(videoImg));
    }

    if (saveVideo)
    {
        video->write(origVideoMat);
    }
}


// Save video data into .avi file
void VideoThread::setVideoName(QString filename)
{
    this->filename = filename;
}

void VideoThread::startSavingVideo()
{
    mutex.lock();
    saveVideo = true;
    video = new VideoWriter(filename.toStdString(),CV_FOURCC('M','J','P','G'),frame_rate, Size(frame_width,frame_height),true);
    mutex.unlock();
}

void VideoThread::stopSavingVideo()
{
    mutex.lock();
    saveVideo = false;
    video->release();
    mutex.unlock();
}


// Show video
void VideoThread::startEmittingVideo()
{
    showVideo = true;
}

void VideoThread::stopEmittingVideo()
{
    showVideo = false;

    // Emit a static icon
    emit processedImage(QPixmap(":/video/video_icon.jpg").scaled(videoMat.cols, videoMat.rows));
}


// Stop thread
void VideoThread::Stop()
{
    stop = true;
    saveVideo = false;
    showVideo = false;

    if(video) {
        video->release();
    }

    if(camera.isOpened()) {
        camera.release();
    }

}

bool VideoThread::isStopped() const{
    return this->stop;
}


// Destructor
VideoThread::~VideoThread()
{
    mutex.lock();
    stop = true;
    condition.wakeOne();
    mutex.unlock();
    wait();
}
