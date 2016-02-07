#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>

/* *****************
 * VideoThread
 * **************** */

void VideoThread::process()
{
    // Init variables
    saveFrame = false;

    // Setup connections
    connect(&readThread, SIGNAL(cameraInfo(int, int, int, int)), this, SLOT(setCameraInfo(int, int, int, int)));
    connect(this, SIGNAL(stopped()), &readThread, SLOT(stop()), Qt::DirectConnection);
    connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));

    // Start thread that reads video frames continuously
    readThread.start(QThread::HighPriority);
}

void VideoThread::setCameraInfo(int frame_width, int frame_height, int fourCC, int fps){
    this->frame_width   = frame_width;
    this->frame_height  = frame_height;
    this->fourCC        = fourCC;
    this->frame_rate    = fps;
}

void VideoThread::saveVideo(bool save)
{
    mutex.lock();
    saveFrame = save;

    // Release previous video writer if still open
    if (!saveFrame && video.isOpened()) {
        video.release();
    }

    // Clear list of frames
    if (saveFrame) {
        playbackList.clear();
    }

    mutex.unlock();
}

void VideoThread::displayVideo(short mode)
{
    mutex.lock();
    dispMode = mode;
    mutex.unlock();
}

void VideoThread::emitVideo(Mat *frame)
{
    // Convert Mat frame into a QPixmap for video feed
    Mat procFrame;
    cv::cvtColor(*frame, procFrame, CV_BGR2RGB);       // invert BGR to RGB
    videoImg = QImage((uchar*)procFrame.data, procFrame.cols, procFrame.rows, procFrame.step, QImage::Format_RGB888);
    QPixmap videoPixmap = QPixmap::fromImage(videoImg);

    // Handle frame saving during data collection
    if (saveFrame) {
        mutex.lock();
        video.write(*frame);
        playbackList.append(videoPixmap);
        mutex.unlock();
    }

    // Emit proper image based on display mode to video feed in GUI
    if (dispMode == 0) {
        emit processedImage(videoPixmap);
    }
    else if (dispMode == 1) {
        emit processedImage(playbackList.at(playbackIdx));
    }
    else {
        emit processedImage(QPixmap(":/video/video_icon.jpg").scaled(frame_width, frame_height));
    }
}

void VideoThread::updatePlaybackIdx(int idx) {
    mutex.lock();
    playbackIdx = idx;
    mutex.unlock();
}

int VideoThread::getNumOfFramesPlayback() {
    return playbackList.size();
}

void VideoThread::stop()
{
    saveVideo(false);
    emit stopped();
}

void VideoThread::setFilename(QString filename)
{
    if(video.isOpened()) {
        video.release();
    }

    QString formatFilename = filename + "_video.avi";

    video.open(formatFilename.toStdString(), fourCC, frame_rate, Size(frame_width,frame_height), true);
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
    int frame_width  = static_cast<int>(camera.get(CV_CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(camera.get(CV_CAP_PROP_FRAME_HEIGHT));
    int fourCC = CV_FOURCC('M','J','P','G');
    int fps = 30;

    camera.set(CV_CAP_PROP_FPS, fps);   // Set frame per second to camera
    emit cameraInfo(frame_width, frame_height, fourCC, fps);

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
