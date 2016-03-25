#ifndef TTS_GUI_H
#define TTS_GUI_H

#include <QMainWindow>

#define WIN32_LEAN_AND_MEAN // Necessary due to problems with Boost
#include "ReadSensors.h"
#include "Magnet.h"
#include "common.h"
#include "Sensor.h"
#include "sensordisplay.h"
#include "videothread.h"
#include "localization.h"
#include "voicemanager.h"
#include "vfbmanager.h"

#include <QVector>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QaudioBuffer>
#include <QTimer>

#include <QCamera>
#include <QImage>

//#include <qcustomplot.h>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

/* **************************************************** *
 *              Variables                               *
 * **************************************************** */
private:
    Ui::MainWindow *ui;
    Magnet magnet;
    QVector<Sensor*> sensors;
    ReadSensors rs;
    VideoThread video;
    Localization loca;

    // Mode selection
    enum Mode { NO_VFB, VFB_REF, VFB_SUB };
    Mode mode;
    VfbManager vfbManager;

    // Audio
    VoiceManager voice;

    struct Waveform{
        QCPGraph *graph;
        QCPRange keysRange;
        QCPRange valuesRange;
    };

    Waveform waveform;

    // Utterances
    QList<QString> classUtter;
    QList<QStringList*> utter;
    int numTrials;
    bool sessionCompleted;

    // Output File locations
    QString experiment_root;
    QString experiment_class;
    QString experiment_utter;
    QString subOutPath;
    QString emfFile;

    // Display sensor UI
    SensorDisplay *sensorUi;

    // Display Tongue Trajectory
    struct LocaTrajPlot {
        QCPCurve *graph;    // Keep the historic of tongue movement
        QCPGraph *leadDot;  // disc tracer of current magnet position
        QCPAxisRect *axis;
    };

    LocaTrajPlot locaTrajPlots[3];

    struct LocaTimePlot {
        QCPGraph *graph;
        QCPAxisRect *axis;
    };

    LocaTimePlot locaTimePlots[3];



/* **************************************************** *
 *              Methods                                 *
 * **************************************************** */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);


private slots:
    void on_configButton_clicked();
    void on_measureEMFButton_clicked();
    void on_startStopTrialButton_toggled(bool checked);
    void on_classBox_currentIndexChanged(int index);
    void on_utteranceBox_currentIndexChanged(int index);
    void updateWaveform(AudioSample sample);
    void on_showMagButton_toggled(bool checked);
    void sensorDisplayClosed();
    void saveEMF();
    void on_trialBox_currentIndexChanged(int index);
    void updateTongueTraj(LocaData locaData);
    void videoManager();


    void on_vfbActivationCheckBox_toggled(bool checked);

    void on_vfbSubModeRadio_toggled(bool checked);

    void on_playRefButton_clicked();

private:
    void loadConfig();
    void loadCalibration(QString calibFilename);
    void setupExperiment();
    void loadExperimentFile(QString experimentFile);
    void setVideo();
    void setAudio();
    void setTongueTraj();
    void clearPlots();
    void setWaveform();
    void beginTrial();
    void stopTrial();
    void setFilePath();
    QVector<double> parseVector(QString myString, bool matrix);
    QString parseUtter(QString rawUtter);

/* ****************************
 *              Signals
 * *************************** */
signals:
    void save(bool);
    void subOutPathSig(QString);
    void stopRecording();
    void videoMode(short);
};

#endif // TTS_GUI_H
