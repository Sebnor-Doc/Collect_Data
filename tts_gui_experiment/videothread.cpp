#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>
#include <QThread>

VideoThread::VideoThread()
{ 
    bool camFound = setCamera();

    if (!camFound) {
        // Provide a better error handling code if camera cannot be found
        qDebug() << "ERROR: TTS camera cannot be found" << endl;
    }

    stopExec = false;
}

bool VideoThread::setCamera()
{
    // Find index of LifeCam webcam
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

    // Open LifeCam if found, o.w return false
    if (cameraIdx == -1) {
        return false;

    } else {
        camera.open(cameraIdx);
        frame_width  = camera.get(CV_CAP_PROP_FRAME_WIDTH);
        frame_height = camera.get(CV_CAP_PROP_FRAME_HEIGHT);
        frame_rate = 30;
        qDebug() << "OK: Camera found as " << cameras.at(cameraIdx).description() << "\n";
        return true;
    }
}


// Slots
void VideoThread::process()
{  
    while (!stopExec) {

        bool readSuccessful = camera.read(videoMat);

        if (!readSuccessful || videoMat.empty()) {          // if we did not get a frame
            qDebug() << "ERROR: CANNOT READ VIDEO IMAGES";
            return;
        }
        else {
            emit newFrame(videoMat);
        }
    }
}

void VideoThread::savingVideo(Mat &frame)
{
    mutex.lock();
    video.write(frame);
    mutex.unlock();
}


void VideoThread::setFilename(QString filename)
{
    if(video.isOpened()) {
        video.release();
    }

    QString formatFilename = filename + "_video.avi";
    video.open(formatFilename.toStdString(), CV_FOURCC('M','J','P','G'), frame_rate, Size(frame_width,frame_height), true);
}

void VideoThread::saveVideo(bool save)
{
    if (save) {
        connect(this, SIGNAL(newFrame(Mat&)), this, SLOT(savingVideo(Mat&)), Qt::DirectConnection);
    }
    else {
        disconnect(this, SIGNAL(newFrame(Mat&)), this, SLOT(savingVideo(Mat&)));
        mutex.lock();
        video.release();
        mutex.unlock();
    }

}

void VideoThread::displayVideo(bool disp)
{
    if (disp) {
        connect(this, SIGNAL(newFrame(Mat&)), this, SLOT(emitVideo(Mat&)),Qt::DirectConnection);
    }
    else {
        disconnect(this, SIGNAL(newFrame(Mat&)), this, SLOT(emitVideo(Mat&)));

        // Emit a static icon
        emit processedImage(QPixmap(":/video/video_icon.jpg").scaled(videoMat.cols, videoMat.rows));
    }
}

void VideoThread::emitVideo(Mat &frame)
{
    Mat procFrame;
    cv::cvtColor(frame, procFrame, CV_BGR2RGB);       // invert BGR to RGB
    videoImg = QImage((uchar*)procFrame.data, procFrame.cols, procFrame.rows, procFrame.step, QImage::Format_RGB888);

    emit processedImage(QPixmap::fromImage(videoImg));
}

// Stop thread
void VideoThread::stop()
{
    mutex.lock();
    stopExec = true;
    mutex.unlock();

    emit finished();
}

// Destructor
VideoThread::~VideoThread()
{
    mutex.lock();
    if (video.isOpened()) {
        video.release();
    }

    if(camera.isOpened()) {
        camera.release();
    }

    mutex.unlock();
}
