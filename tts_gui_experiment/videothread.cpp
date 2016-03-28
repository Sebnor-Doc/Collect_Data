#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>

/* *****************
 * VideoThread
 * **************** */

void VideoThread::process()
{
    // Setup connections
    connect(&readThread, SIGNAL(cameraInfo(int, int, int, int)), this, SLOT(setCameraInfo(int, int, int, int)));
    connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
    connect(this, SIGNAL(stopped()), &readThread, SLOT(stop()), Qt::DirectConnection);

    // Start thread that reads video frames continuously
    readThread.start(QThread::HighPriority);
}

void VideoThread::setCameraInfo(int frame_width, int frame_height, int fourCC, int fps){
    this->frame_width   = frame_width;
    this->frame_height  = frame_height;
    this->fourCC        = fourCC;
    this->frame_rate    = fps;
}

void VideoThread::saveVideo(bool saveVal)
{
    mutex.lock();

    if (saveVal) {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(saveFrame(Mat*)));
    }
    else {
        disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(saveFrame(Mat*)));
        video.release();
    }

    mutex.unlock();
}

void VideoThread::displayVideo(VideoMode mode)
{
    mutex.lock();

    if(mode == LIVE_FEED) {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
    }

    else if (mode == REPLAY_SUB || mode == REPLAY_REF) {
        disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));

        int upperFrameIdx = static_cast<int>(replayVideo.get(CAP_PROP_FRAME_COUNT)) - 1;
        qDebug() << "upperFrameIdx: " << upperFrameIdx;

        if (mode == REPLAY_SUB) {
            emit replayFrameRange(0, upperFrameIdx);
            updatePlaybackIdx(0);
        }

        else if (mode == REPLAY_REF) {

            int fps = frame_rate;
            int waitBtwFrames = static_cast<int>(1000.0 / fps);         // Wait between Frames in Milliseconds

            for (int frameIdx = 0; frameIdx <= upperFrameIdx; frameIdx++) {

                updatePlaybackIdx(frameIdx);
                QThread::msleep(waitBtwFrames);
            }
        }
    }

    else if (mode == NO_FEED) {
        disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
        emit processedImage(QPixmap(":/video/video_icon.jpg").scaled(frame_width, frame_height));
    }

    else {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
        qDebug() << "ERROR: displayVideo arg is not valid";
    }

    mutex.unlock();
}

void VideoThread::emitVideo(Mat *frame)
{
    // Convert Mat frame into a QPixmap for video feed
    Mat procFrame;
    cv::cvtColor(*frame, procFrame, CV_BGR2RGB);       // invert BGR to RGB
    QImage videoImg = QImage((uchar*)procFrame.data, procFrame.cols, procFrame.rows, procFrame.step, QImage::Format_RGB888);
    QPixmap videoPixmap = QPixmap::fromImage(videoImg);

    emit processedImage(videoPixmap);
}

void VideoThread::saveFrame(Mat *frame){
    mutex.lock();
    video.write(*frame);
    mutex.unlock();
}

void VideoThread::updatePlaybackIdx(int idx) {

    bool success = replayVideo.set(CAP_PROP_POS_FRAMES, idx);

    if (success) {
        Mat replayFrame;
        replayVideo >> replayFrame;

        cv::cvtColor(replayFrame, replayFrame, CV_BGR2RGB);       // invert BGR to RGB
        QImage replayImg  = QImage((uchar*)replayFrame.data, replayFrame.cols, replayFrame.rows, replayFrame.step, QImage::Format_RGB888);
        QPixmap replayPixmap = QPixmap::fromImage(replayImg);

        emit processedImage(replayPixmap);
    }
}

void VideoThread::setSubFilename(QString filename)
{
    subFilePath = filename + "_video.avi";
    video.open(subFilePath.toStdString(), fourCC, frame_rate, Size(frame_width,frame_height), true);
}

void VideoThread::setReplay(QString rootPath)
{

    replayFilePath = rootPath + "_video.avi";
    replayVideo.open(replayFilePath.toStdString());
}

void VideoThread::stop()
{
    saveVideo(false);
    emit stopped();
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
    int fourCC = CV_FOURCC('D','I','V','3');
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
