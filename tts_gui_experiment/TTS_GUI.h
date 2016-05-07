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
#include "patientdialog.h"

#include <QVector>
#include <QAudioRecorder>
#include <QAudioProbe>
#include <QaudioBuffer>
#include <QTimer>
#include <QThread>

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
    Localization loca;

    // Mode selection
    enum Mode { NO_VFB, REF_VFB, SUB_VFB, SUB_NO_SCORE };
    Mode mode;

    VfbManager  *vfbManager = 0;
    PatientDialog *patientDialog = 0;

    // Audio
    VoiceManager voice;

    struct Waveform{
        QCPGraph *graph;
        QCPGraph *refGraph;
        QCPRange keysRange;
        QCPRange valuesRange;
    };

    Waveform waveform;

    // Utterances
    QList<QString> classUtter;
    QList<QStringList*> utter;
    int numTrials;
    int currentTrial;
    bool sessionCompleted;

    // Output File locations
    QString experiment_root;
    QString experiment_class;
    QString experiment_utter;
    QString subOutPath;
    QString subOutPathReplay;
    QString emfFile;

    // Display sensor UI
    SensorDisplay *sensorUi;

    // Display Tongue Trajectory
    struct LocaTrajPlot {
        QCPCurve *graph;    // Keep the historic of tongue movement
        QCPCurve *refCurve;
        QCPGraph *leadDot;  // disc tracer of current magnet position
        QCPAxisRect *axis;
    };

    LocaTrajPlot locaTrajPlots[3];

    struct LocaTimePlot {
        QCPGraph *graph;
        QCPGraph *refGraph;
        QCPAxisRect *axis;
        QCPRange *timeRange;
        QCPRange *YRange;
    };

    LocaTimePlot locaTimePlots[3];

    // Display video
    VideoThread video;
    QCPCurve *lipsCurve;
    QCPCurve *lipsBox;

/* **************************************************** *
 *              Methods                                 *
 * **************************************************** */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);


private slots:
    void sensorDisplayClosed();
    void saveEMF();

    void on_configButton_clicked();
    void on_measureEMFButton_clicked();
    void on_reuseEMFButton_clicked();
    void on_startStopTrialButton_toggled(bool checked);
    void on_classBox_currentIndexChanged(int index);
    void on_utteranceBox_currentIndexChanged(int index);
    void on_showMagButton_toggled(bool checked);
    void on_trialBox_currentIndexChanged(int index);
    void on_vfbActivationCheckBox_toggled(bool checked);
    void on_vfbSubModeRadio_toggled(bool checked);
    void on_playRefButton_clicked();
    void playRefFinished();
    void on_vfbRefModeRadio_toggled(bool checked);
    void on_vfbSubNoVfbModeRadio_toggled(bool checked);

    void updateTongueTraj(LocaData locaData);
    void updateRefTongueTraj();
    void updateWaveform(AudioSample sample, bool ref = false);
    void updateVideoFeedImage(const QPixmap &image);
    void updateTrackLipsFeed(QVector<QPoint> lipsPos);
    void videoManager();
    void scoreGenerated();


private:
    void loadConfig();
    void loadCalibration(QString calibFilename);
    void setupExperiment();
    void loadExperimentFile(QString experimentFile);
    void beginTrial();
    void stopTrial();
    void setVideo();
    void setVideoPlayer();
    void setAudio();
    void setTongueTraj();
    void clearPlots();
    void setWaveform();
    void setFilePath();
    void updateButtonsAfterEMF();
    QVector<double> parseVector(QString myString, bool matrix);
    QString parseUtter(QString rawUtter);
    void showVfbWidgets(bool isVfbSub);
    QString formatUtterance(QString rawUtter);

/* ****************************
 *              Signals
 * *************************** */
signals:
    void save(bool);
    void subOutPathSig(QString);
    void stopRecording();
    void videoMode(VideoMode);
    void computeScoreSig();
    void utterSig(QString);
};

#endif // TTS_GUI_H
