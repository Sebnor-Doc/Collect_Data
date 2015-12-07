#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>

/* *****************
 * VideoThread
 * **************** */
void VideoThread::process()
{
    connect(&readThread, SIGNAL(cameraInfo(int, int)), this, SLOT(setCameraInfo(int, int)));
    connect(this, SIGNAL(stopped()), &readThread, SLOT(stop()), Qt::DirectConnection);
//    connect(this, SIGNAL(stopped()), this, SLOT(deleteLater()));

    readThread.start(QThread::HighPriority);
}

void VideoThread::setCameraInfo(int frame_width, int frame_height){
    this->frame_width  = frame_width;
    this->frame_height = frame_height;
    frame_rate = 30;
}

void VideoThread::savingVideo(Mat *frame)
{
    mutex.lock();
    video.write(*frame);
    mutex.unlock();
}

void VideoThread::saveVideo(bool save)
{
    if (save) {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(savingVideo(Mat*)));
    }
    else {
        disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(savingVideo(Mat*)));

        mutex.lock();

        if (video.isOpened()) {
            video.release();
        }

        mutex.unlock();
    }
}

void VideoThread::displayVideo(bool disp)
{
    if (disp) {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
    }
    else {
        disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));

        // Emit a static icon
        emit processedImage(QPixmap(":/video/video_icon.jpg").scaled(frame_width, frame_height));
    }
}

void VideoThread::emitVideo(Mat *frame)
{
    Mat procFrame;
    cv::cvtColor(*frame, procFrame, CV_BGR2RGB);       // invert BGR to RGB
    videoImg = QImage((uchar*)procFrame.data, procFrame.cols, procFrame.rows, procFrame.step, QImage::Format_RGB888);

    emit processedImage(QPixmap::fromImage(videoImg));
}

void VideoThread::stop()
{
    mutex.lock();
    displayVideo(false);
    saveVideo(false);
    mutex.unlock();

    emit stopped();
}

void VideoThread::setFilename(QString filename)
{
    if(video.isOpened()) {
        video.release();
    }

    QString formatFilename = filename + "_video.avi";
    video.open(formatFilename.toStdString(), CV_FOURCC('M','J','P','G'), frame_rate, Size(frame_width,frame_height), true);
}

VideoThread::~VideoThread(){
    emit finished();
}

/* *****************
 * VideoReadWorker
 * **************** */
VideoReadWorker::VideoReadWorker() {
    // Start execution of video recording
    stopExec = false;

    // Find index of TTS supported camera in the list of connected cameras
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    int cameraIdx = -1;

    for (int i = 0; i < cameras.size(); i++) {
        // Get the name of the camera
        QString cameraName = cameras.at(i).description();

        // Search if name matches one of TTS camera
        if (cameraName.contains("LifeCam") || cameraName.contains("PC CAMERA")) {
            cameraIdx = i;
            break;
        }
    }

    // Open Camera if found
    if (cameraIdx == -1) {
        // Find a good resolution to camera not found
        qDebug() << "ERROR: No camera found";

    } else {
        camera.open(cameraIdx);
        qDebug() << "OK: Camera found as " << cameras.at(cameraIdx).description() << "\n";
    }
}

void VideoReadWorker::run(){
    // Send info needed for saving video data into a file
    int frame_width  = camera.get(CV_CAP_PROP_FRAME_WIDTH);
    int frame_height = camera.get(CV_CAP_PROP_FRAME_HEIGHT);
    emit cameraInfo(frame_width, frame_height);

    // Read video frames from camera
    while(!stopExec) {
        mutex.lock();
        bool readSuccessful = camera.read(frame);

        if (readSuccessful) {
            emit newFrame(&frame);
        }
        mutex.unlock();
    }

    // Closing procedure
    camera.release();
    qDebug() << "\n -- Camera Released\n";
}

void VideoReadWorker::stop(){
    mutex.lock();
    stopExec = true;
    mutex.unlock();
}

VideoReadWorker::~VideoReadWorker(){

    if(camera.isOpened()) {
        camera.release();
    }
}
