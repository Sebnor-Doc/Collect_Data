#include "VideoThread.h"
#include <QCameraInfo>
#include <QPixmap>
#include <qmath.h>

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
    this->mode = mode;

    disconnect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));

    if(mode == RAW_FEED || mode == LIP_CONTOUR || mode == BW_FEED ) {
        connect(&readThread, SIGNAL(newFrame(Mat*)), this, SLOT(emitVideo(Mat*)));
    }


    else if (mode == REPLAY_SUB || mode == REPLAY_REF) {

        int upperFrameIdx = static_cast<int>(replayVideo.get(CAP_PROP_FRAME_COUNT)) - 1;

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
        emit processedImage(QPixmap(":/video/video_icon.jpg"));
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

    if (mode == RAW_FEED) {
        QImage videoImg = QImage((uchar*)procFrame.data, procFrame.cols, procFrame.rows, procFrame.step, QImage::Format_RGB888);
        QPixmap videoPixmap = QPixmap::fromImage(videoImg);
        emit processedImage(videoPixmap);
    }

    if (mode != RAW_FEED) {
        trackLips(procFrame);
    }
}

/* Track Lips */
void VideoThread::trackLips(cv::Mat &frame)
{
    // Lower frame resolution to reduce execution time
    cv::resize(frame, frame, Size(320, 240), 0, 0, INTER_AREA);

    // Process frame to extract a lips into a binary image
    Mat bwFrame = extractLipsAsBWImg(frame);

    if (mode == BW_FEED) {
        QImage bwImg = QImage((uchar*)bwFrame.data, bwFrame.cols, bwFrame.rows, bwFrame.step, QImage::Format_Grayscale8);
        QPixmap bwPixmap = QPixmap::fromImage(bwImg);
        emit processedImage(bwPixmap);
        return;
    }

    // Emit raw frame as a background for lips curve
    QImage frameImg = QImage((uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    QPixmap framePixmap = QPixmap::fromImage(frameImg);
    emit processedImage(framePixmap);

    // Process binary image to localize points on the lip boundaries
    QVector<QPoint> lipsPoints  = extractPointsOnLipsEdge(bwFrame);
    emit lipPosition(lipsPoints);
}

Mat VideoThread::extractLipsAsBWImg(Mat &frame)
{
    // Create a copy of frame with float as data type
    // Needed for Red color extraction algorithm
    Mat formattedFrame(frame.rows, frame.cols, CV_32FC3);
    frame.convertTo(formattedFrame, CV_32FC3, 1.0/255.0);

    // Split the image into different color channels
    std::vector<Mat> rgbChannels;
    cv::split(formattedFrame, rgbChannels);

    Mat redChannel = rgbChannels[0];
    Mat greenChannel = rgbChannels[1];

    // Apply the lips extraction filter based on Red pixels differentiation
    Mat bwFrame(frame.rows, frame.cols, CV_32FC1);
    cv::add(greenChannel, 0.000001, bwFrame);
    cv::divide(redChannel, bwFrame, bwFrame);
    cv::log(bwFrame, bwFrame);

    // Compute the threshold to render lips-like area to white and other areas to black
    Mat frameVect;
    bwFrame.reshape(0, 1).copyTo(frameVect);            // Flatten out the frame into a row vector
    cv::sort(frameVect, frameVect, CV_SORT_ASCENDING);
    double thres_coeff = 0.18;                          // Variable that sets strength of discrimination (lower = more discrimination)
    int threshIdx = (frameVect.cols - 1) - qFloor(frameVect.cols * thres_coeff);
    float thresVal = frameVect.at<float>(0, threshIdx);

    // Create the binary image
    Mat bwFrameProc = bwFrame > thresVal;

    // Keep only the biggest agglomerate of white pixels as more likely related to lips
    Mat connCompLabels, connCompStats, connCompCentroids;
    cv::connectedComponentsWithStats(bwFrameProc, connCompLabels, connCompStats, connCompCentroids, 8, CV_16U);

    int widerConnComp[2] = {0 , 0};                     // Format: (label , numPixels)

    for (int i = 1; i < connCompStats.rows; i++) {      // Start from 1 to ignore background (black pixels)

        int numPixels = static_cast<int>(connCompStats.at<char32_t>(i, 4));

        if (numPixels >= widerConnComp[1]) {
            widerConnComp[0] = i;
            widerConnComp[1] = numPixels;
        }
    }

    Mat bwFrameFiltered = (connCompLabels == widerConnComp[0]);

    // Return a binary image with only the lips as white pixels
    return bwFrameFiltered;
}

QVector<QPoint> VideoThread::extractPointsOnLipsEdge(Mat &binaryImg)
{
    // Two data structures are needed as 2 points exists for a same column
    QVector<QPoint> upperLipPts;
    QVector<QPoint> lowerLipPts;

    // Skip columns to reduce execution time
    int colsDownSampling = 50;
    int numColsPerScan = binaryImg.cols / colsDownSampling;

    // Scan each selected columns
    for (int colIdx = 0; colIdx < binaryImg.cols; colIdx += numColsPerScan) {

        bool upperLipFound = false;
        bool lowerLipFound = false;
        QPoint lowerPoint;

        // Scan each row
        for (int rowIdx = 0; rowIdx < binaryImg.rows; rowIdx++) {

            int pixelIntensity = static_cast<int>(binaryImg.at<uchar>(rowIdx, colIdx));

            // Append first point where black pixel changes to white (upper lip)
            if ( pixelIntensity == 255 && !upperLipFound) {
                upperLipPts.append(QPoint(colIdx, rowIdx));
                upperLipFound = true;
            }

            // Create a point at the location where a white pixel changes to black (lower lip)
            else if ( pixelIntensity == 0 && upperLipFound && !lowerLipFound ) {
                lowerPoint.setX(colIdx);
                lowerPoint.setY(rowIdx);
                lowerLipFound = true;
            }

            // Manages cases where a black patch of pixels exists between upper and lower lips
            else if (lowerLipFound && pixelIntensity == 255) {
                lowerLipFound = false;
            }
        }

        // Add lower point if found
        if(!lowerPoint.isNull()) {
            lowerLipPts.push_front(lowerPoint); // Push to front to make line creation easier
        }

        // Add pixel of last row as lower lip if not found
        if (upperLipFound && !lowerLipFound) {
            lowerLipPts.push_front(QPoint(colIdx, binaryImg.rows - 1));
        }
    }


    QVector<QPoint> lipsPoints;

    foreach (QPoint point, upperLipPts) {
        lipsPoints.append(point);
    }

    foreach (QPoint point, lowerLipPts) {
        lipsPoints.append(point);
    }

    return lipsPoints;
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
