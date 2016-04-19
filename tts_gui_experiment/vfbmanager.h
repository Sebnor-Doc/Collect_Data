#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
#include <typedef.h>
#include <QVector>
#include <typedef.h>
#include <QAudioDecoder>
#include <QThread>
#include <QSoundEffect>

class ScoreGen : public QObject
{
    Q_OBJECT

public:
    ScoreGen(QObject *parent = 0);
    ~ScoreGen();

public slots:
    void startMatlabEngine();
    void computeScore(RefSubFilePaths paths);

signals:
    void scoreSig(Scores scores, RefSubFilePaths info);
};


class VfbManager : public QObject
{
    Q_OBJECT

private:
    QString refRootPath;
    QString subRootPath;
    RefSubFilePaths paths;

    QAudioDecoder *audioFile;
    double audioStartTime;
    const short ADR = 100;

    QSoundEffect *refVoiceReplay = 0;

    QThread *scoreGenThread = 0;
    ScoreGen *scoreGen = 0;


public:
    VfbManager(QObject *parent = 0);
    ~VfbManager();

    void setRootPath(QString refRoot, QString subRoot);
    void setPaths(RefSubFilePaths paths);

    QString getRefRootPath();
    RefSubFilePaths getPaths();
    QVector<LocaData> getRefLocaData();
    void getAudioSample();
    void playAudio();



public slots:
    void readAudioBuffer();
    void computeScores();
    void saveScores(Scores scores, RefSubFilePaths info);

private slots:
    void handleRefVoiceReplayState();

signals:
    void audioSampleSig(AudioSample sample, bool ref);
    void computeScoreSig(RefSubFilePaths paths);
    void scoreSig(Scores scores);
    void voiceReplayFinished();

};

#endif // VFBMANAGER_H
