#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
#include <typedef.h>
#include <QVector>
#include <typedef.h>
#include <QAudioDecoder>

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


public slots:
    void startVFBProgram();
    void readAudioBuffer();
    void computeScores();

signals:
    void audioSampleSig(AudioSample sample, bool ref);
    void scoreSig(Scores scores);

};

#endif // VFBMANAGER_H
