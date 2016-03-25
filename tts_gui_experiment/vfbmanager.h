#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
#include <QProcess>
#include <QVector>
#include <typedef.h>
#include <QAudioDecoder>

class VfbManager : public QObject
{
    Q_OBJECT

public:
    VfbManager(QObject *parent = 0);

    void setVfbFilePath(QString filePath);
    void setRefRootPath(QString rootPath);
    void setRefOutPath(QString relativePath);

    void setSubId(int id);
    void setNumTrials(int numTrials);

    void setCurrentUtter(QString utter);
    void setCurrentTrial(int trial);

    void updateXML(int selection);
    void startVFBProgram();
    void playAudio();
    QVector<LocaData> getRefLocaData();
    void getAudioSample();

    QString getRefOutPath();


private:
    QString vfbFilePath;
    QString refRootPath;
    QString refOutPath;
    QString subOutPath;

    int currentTrial;
    QString currentUtter;

    int subId;
    int numTrials;

    QProcess *scoreGenProg;

    QAudioDecoder *audioFile;
    double audioStartTime;
    const short ADR = 100;

public slots:
    void setSubOutPath(QString path);
    void readAudioBuffer();

signals:
    void audioSample(AudioSample sample, bool ref);

};

#endif // VFBMANAGER_H
