#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
#include <typedef.h>
#include <QVector>
#include <typedef.h>
#include <QAudioDecoder>
#include <QThread>

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
    void scoreSig(Scores scores);
};


class VfbManager : public QObject
{
    Q_OBJECT

public:
    VfbManager(QObject *parent = 0);
    ~VfbManager();

    void setRefRootPath(QString rootPath);
    void setPaths(RefSubFilePaths paths);

    QString getRefRootPath();
    RefSubFilePaths getPaths();
    QVector<LocaData> getRefLocaData();
    void getAudioSample();
    void playAudio();

private:
    QString refRootPath;
    RefSubFilePaths paths;

    QAudioDecoder *audioFile;
    double audioStartTime;
    const short ADR = 100;

    QThread *scoreGenThread = 0;
    ScoreGen *scoreGen = 0;

public slots:
    void readAudioBuffer();
    void computeScores();

signals:
    void audioSampleSig(AudioSample sample, bool ref);
    void computeScoreSig(RefSubFilePaths paths);
    void scoreSig(Scores scores);

};

#endif // VFBMANAGER_H
