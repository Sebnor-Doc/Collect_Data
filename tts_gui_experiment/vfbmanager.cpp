#include "vfbmanager.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QSound>
#include <QCoreApplication>

/* Matlab Headers */
#include "LocalizationScore.h"
#include "AudioScore.h"
#include "MagneticScore.h"
#include "VideoScore.h"

// Audio
#include <QAudioFormat>
#include <QAudioBuffer>


VfbManager::VfbManager(QObject *parent) : QObject(parent)
{}


void VfbManager::startVFBProgram()
{
    mclmcrInitialize();

    bool mclInitApp         = mclInitializeApplication(NULL, 0);
    bool locaInitSuccess    = LocalizationScoreInitialize();
    bool audioInitSuccess   = AudioScoreInitialize();
    bool magInitSuccess     = MagneticScoreInitialize();
    bool videoInitSuccess   = VideoScoreInitialize();

    qDebug() << QString("Matlab DLLs Init. Status: MCL = %1 , Loca = %2 , Audio = %3 , Mag = %4 , Video = %5")
                .arg(mclInitApp).arg(locaInitSuccess).arg(audioInitSuccess).arg(magInitSuccess).arg(videoInitSuccess);
}

void VfbManager::setRefRootPath(QString rootPath)
{
    refRootPath = rootPath;
}

void VfbManager::setPaths(RefSubFilePaths paths)
{
    this->paths = paths;
}

QString VfbManager::getRefRootPath()
{
    return refRootPath;
}

RefSubFilePaths VfbManager::getPaths()
{
    return paths;
}

QVector<LocaData> VfbManager::getRefLocaData()
{
    QVector<LocaData> refLocaData;

    QFile input(paths.refLoca);
    QTextStream in(&input);

    if (!input.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open reference trajectory file located at:\n" << paths.refLoca << endl;
        return refLocaData;
    }

    while(!in.atEnd())
    {
        QStringList locaSample = in.readLine().split(" ");

        // Convert data
        LocaData locaData;
        locaData.x      = locaSample.at(0).toDouble() * 100.0;
        locaData.y      = locaSample.at(1).toDouble() * 100.0;
        locaData.z      = locaSample.at(2).toDouble() * 100.0;
        locaData.theta  = locaSample.at(3).toDouble() * 1.0;
        locaData.phi    = locaSample.at(4).toDouble() * 1.0;
        locaData.time   = locaSample.at(5).toDouble() / 1000.0;

        refLocaData.append(locaData);
    }

    input.close();
    return refLocaData;
}


/* Audio Control */

void VfbManager::playAudio()
{
    // Verify if reference sound file exist
    QFileInfo checkFile(paths.refAudio);

    if (checkFile.exists() && checkFile.isFile()) {
        QSound::play(paths.refAudio); // Play sound
    }

    else
    {
        QString errorMsg = "ERROR: Cannot found following audio file:\n" + paths.refAudio;
        qDebug() << errorMsg;
    }
}

void VfbManager::getAudioSample()
{
    audioFile = new QAudioDecoder(this);
    audioFile->setSourceFilename(paths.refAudio);

    connect(audioFile, SIGNAL(bufferReady()), this, SLOT(readAudioBuffer()));
    connect(audioFile, SIGNAL(finished()), audioFile, SLOT(stop()));

    audioStartTime = 0.0;
    audioFile->start();
}

void VfbManager::readAudioBuffer()
{
    QAudioBuffer audioBuffer = audioFile->read();

    // PeakValue is used to normalize audio samples to [-1,1]
    qreal peakValue;

    // peak value changes according to sample size
    if (audioBuffer.format().sampleSize() == 32)
      peakValue=INT_MAX;
    else if (audioBuffer.format().sampleSize() == 16)
      peakValue=SHRT_MAX;
    else
      peakValue=CHAR_MAX;

    // Get an array that points to buffer data
    // qint16 is chosen as per buffer.format.sampleType returns signed int of 16 bits
    const qint16 *data = audioBuffer.constData<qint16>();

    // Using vector to avoid adding each sample to waveform
    // It is more efficient to update waveform at once with all data
    AudioSample sample;

    // Downsample by ADR value, o.w consuming too much resources
    double period = (1.0/audioBuffer.format().sampleRate());

    for(int i=0; i< audioBuffer.frameCount(); i += ADR){

        double dataNorm = data[i] / peakValue;
        double time = audioStartTime + (i * period);
        sample.insert(time, dataNorm);
    }

    // Update starting point for next sample
    audioStartTime += audioBuffer.duration() / 1000000.0 ; // convert from microseconds to seconds

    emit audioSampleSig(sample, true);
}

void VfbManager::computeScores()
{
    QString trainModelQStr      = QString(QCoreApplication::applicationDirPath() + "/trainedModel.mat");
    QString uniformLBP8QStr     = QString(QCoreApplication::applicationDirPath() + "/UniformLBP8.txt");

    // Convert QString type to Matlab mwArray type
    mwArray refTrajFile(paths.refLoca.toLatin1().constData());
    mwArray subTrajFile(paths.subLoca.toLatin1().constData());

    mwArray refAudioFile(paths.refAudio.toLatin1().constData());
    mwArray subAudioFile(paths.subAudio.toLatin1().constData());

    mwArray refMagFile(paths.refMag.toLatin1().constData());
    mwArray subMagFile(paths.subMag.toLatin1().constData());

    mwArray refVideoFile(paths.refLips.toLatin1().constData());
    mwArray subVideoFile(paths.subLips.toLatin1().constData());
    mwArray videoTrainingModelFile(trainModelQStr.toLatin1().constData());
    mwArray videoUniformLBP8File(uniformLBP8QStr.toLatin1().constData());

    // Perform score generation
    Scores scores;

    mwArray locaScoreML;
    locaScoreMain(1, locaScoreML, refTrajFile, subTrajFile, refAudioFile, subAudioFile);
    scores.loca = locaScoreML(1,1);

    mwArray audioScoreML;
    audioScoreMain(1, audioScoreML, refAudioFile, subAudioFile);
    scores.voice = audioScoreML(1,1);

    mwArray magScoreML;
    magneticScoreMain(1, magScoreML, refMagFile, subMagFile, refAudioFile, subAudioFile);
    scores.mag = magScoreML(1,1);

    mwArray videoScoreML;
    videoScoreMain(1, videoScoreML, videoTrainingModelFile, videoUniformLBP8File, refVideoFile, subVideoFile);
    scores.lips = videoScoreML(1,1);

    emit scoreSig(scores);
}


VfbManager::~VfbManager()
{
    LocalizationScoreTerminate();
    AudioScoreTerminate();
    MagneticScoreTerminate();
    VideoScoreTerminate();
    mclTerminateApplication();
    qDebug() << "Matlab Runtime Ended";
}
