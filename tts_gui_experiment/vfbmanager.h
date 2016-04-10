#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
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
    void setRefOutPath(QString relativePath);

    void setSubId(int id);
    void setNumTrials(int numTrials);

    void setCurrentUtter(QString utter);
    void setCurrentTrial(int trial);


    void playAudio();
    QVector<LocaData> getRefLocaData();
    void getAudioSample();

    QString getRefOutPath();


private:
    QString refRootPath;
    QString refOutPath;
    QString subOutPath;

    int currentTrial;
    QString currentUtter;

    int subId;
    int numTrials;


    QAudioDecoder *audioFile;
    double audioStartTime;
    const short ADR = 100;

public slots:
    void startVFBProgram();
    void setSubOutPath(QString path);
    void readAudioBuffer();

signals:
    void audioSample(AudioSample sample, bool ref);

};

#endif // VFBMANAGER_H
