#ifndef VOICEMANAGER_H
#define VOICEMANAGER_H

#include <common.h>
#include <QObject>
#include <QMap>
#include <QAudioRecorder>
#include <QAudioBuffer>
#include <QAudioProbe>



class VoiceManager : public QObject
{
    Q_OBJECT

private:
    const short ADR = 100;
    QAudioRecorder *audio1;
    QAudioRecorder *audio2;
    QAudioProbe *probe;
    qreal peakValue;
    double startTime;

public:
    explicit VoiceManager(QObject *parent = 0);

public slots:
    void init();
    void setFilename(QString filename);
    void save(bool saveVal);
    void stop();

private slots:
    void processBuffer(QAudioBuffer audioBuffer);

signals:
    void audioSample(AudioSample sample);

};

#endif // VOICEMANAGER_H
