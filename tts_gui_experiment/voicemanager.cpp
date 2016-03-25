#include "voicemanager.h"
#include <QUrl>

VoiceManager::VoiceManager(QObject *parent) : QObject(parent){
    startTime = 0;
}

void VoiceManager::init()
{
    audio1 = new QAudioRecorder(this);
    audio2 = new QAudioRecorder(this);

    QStringList audioDevices = audio1->audioInputs();

    foreach (QString device, audioDevices) {

        if (device.contains("USB PnP"))
        {
            if (!audio1->audioInput().contains("USB PnP")) {
                audio1->setAudioInput(device);

            } else if (device.compare(audio1->audioInput()) != 0) {
                audio2->setAudioInput(device);
                break;
            }
        }
    }

    // High Settings
    QAudioEncoderSettings settings;
    settings.setQuality(QMultimedia::VeryHighQuality);
    settings.setEncodingMode(QMultimedia::ConstantBitRateEncoding);
    settings.setSampleRate(44100);

    audio1->setAudioSettings(settings);
    audio2->setAudioSettings(settings);

    qDebug() << "Audio Source 1: " << audio1->audioInput();
    qDebug() << "Audio Source 2: " << audio2->audioInput() << endl;

    probe = new QAudioProbe(this);
    probe->setSource(audio1);

    // Only one probe is used to update waveform as the other one provides similar audio data
    connect(probe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(processBuffer(QAudioBuffer)));
}

void VoiceManager::processBuffer(QAudioBuffer audioBuffer)
{
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
        double time = startTime + (i * period);
        sample.insert(time, dataNorm);
    }

    // Update starting point for next sample
    startTime += audioBuffer.duration() / 1000000.0 ; // convert from microseconds to seconds


    emit audioSample(sample, false);
}

void VoiceManager::setFilename(QString filename)
{
    audio1->setOutputLocation(QUrl::fromLocalFile(filename + "_audio1.wav"));
    audio2->setOutputLocation(QUrl::fromLocalFile(filename + "_audio2.wav"));
}

void VoiceManager::save(bool saveVal)
{
    startTime = 0.0;

    audio1->stop();
    audio2->stop();

    if (saveVal) {
        audio1->record();
        audio2->record();
    }
}

void VoiceManager::stop()
{
    save(false);
}
