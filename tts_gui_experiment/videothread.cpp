#include "VideoThread.h"
#include <QDateTime>
#include <time.h>
#include <QUrl>
#include <QCameraInfo>

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
    startWebcam();
    while (!stop)
    {
        mutex.lock();
        capture.read(matOriginal);
        emitVideo();
        mutex.unlock();
    }
}

void VideoThread::startWebcam()
{
    // Find index of LifeCam webcam
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    int cameraIdx = -1;
    int cameraIter = -1;
    foreach (const QCameraInfo &cameraInfo, cameras) {
        cameraIter++;
        if (cameraInfo.description().contains("LifeCam")) {
            cameraIdx = cameraIter;
            break;
        }
    }

    // Open LifeCam or throw runtime exception if cannot be found
    if (cameraIdx == -1) {
        QString errorMsg = "LifeCam camera cannot be found";
        qDebug() << errorMsg << endl;
        throw std::runtime_error(errorMsg.toStdString());

    } else {
        capture.open(cameraIdx);
        frame_width  = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frame_height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        frame_rate = 30;
    }
}

void VideoThread::emitVideo()
{
    if (saveVideo)
    {
        tempVid->push_back(matOriginal.clone());
    }
    if (showVideo)
    {
        cvtColor(matOriginal, matOriginal, CV_BGR2RGB);
        img = QImage((uchar*) matOriginal.data, matOriginal.cols, matOriginal.rows, matOriginal.step, QImage::Format_RGB888);
        emit processedImage(img);
    }
}


// Stop video recording
void VideoThread::Stop()
{
    stop = true;
    endWebcam();
}

void VideoThread::endWebcam()
{
    capture.release();
}


//Start saving video
void VideoThread::startVideo()
{
    tempVid = new vector<Mat>();
    mutex.lock();
    saveVideo = true;
    start_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
    mutex.unlock();
}

void VideoThread::endVideo()
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
    img = QImage(matOriginal.cols, matOriginal.rows, QImage::Format_RGB888);
    emit processedImage(img);
}


// Put thread to sleep
void VideoThread::msleep(long ms){
    QThread::msleep(ms);
}

void VideoThread::usleep(long us){
    QThread::usleep(us);
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
    for (int i=0; i<newIndices.size(); i++)
    {
        finalVid->push_back(tempVid->at((int)round(newIndices.at(i))));
        video->write(finalVid->at(i));
    }
    video->release();
}

void VideoThread::postProcessVideo(int threshold)
{
    int startIndex = (int) round(threshold*frame_rate/1000.0);
    QString post_filename = filename.replace(".avi","_post.avi");
    video = new VideoWriter(post_filename.toStdString(),CV_FOURCC('M','J','P','G'),frame_rate, Size(frame_width,frame_height),true);
    for (int i=startIndex; i<finalVid->size(); i++)
    {
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


// To be categorized
void VideoThread::setVideoName(QString filename)
{
    this->filename = filename;
}

bool VideoThread::isStopped() const{
    return this->stop;
}


// Destructor
VideoThread::~VideoThread()
{
    mutex.lock();
    stop = true;
    capture.release();
    condition.wakeOne();
    mutex.unlock();
    wait();
}
