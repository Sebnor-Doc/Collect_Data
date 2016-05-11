#ifndef PATIENTDIALOG_H
#define PATIENTDIALOG_H

#include <QDialog>
#include <typedef.h>
#include <qcustomplot.h>

namespace Ui {
class PatientDialog;
}

class PatientDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PatientDialog(QWidget *parent = 0);
    ~PatientDialog();

private:
    Ui::PatientDialog *ui;
    int numTrials;
    VideoMode videoMode;

    // Score plot
    QVector<QCPBars*> locaBars;
    QVector<QCPBars*> magBars;
    QVector<QCPBars*> voiceBars;
    QVector<QCPBars*> lipsBars;
    QVector<QCPBars*> avgScoreBars;
    QCPColorGradient colorGrad;
    int currentTrial = 1;
    int nextTrial    = 1;

    // Reference playing
    QVector<QLabel*> playRefLabels;

    // Audio Waveform
    struct Waveform{
        QCPGraph *graph;
        QCPGraph *refGraph;
        QCPRange keysRange;
        QCPRange valuesRange;
    };

    Waveform waveform;

    // Tongue Tracking
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

public:
    void setScorePlot(int numTrials);
    void setAudioWaveform();
    void setTonguePlot();
    void setCurrentTrial(int trial);
    void setNextTrial(int nextTrial);
    void showScores(bool show);

public slots:
    void updateUtter(QString utter);
    void updateScores(Scores scores);
    void updateVideo(QPixmap image);
    void updateWaveform(AudioSample sample, bool ref = false);
    void updateTongue(LocaData locaData);
    void updateRefTongue(QVector<LocaData> refLocaData);
    void recording(bool isRecording);
    void updateVideoMode(VideoMode videoMode);

private slots:
    void on_bioFeedCheckBox_toggled(bool checked);

private:
    void clearScorePlot();
    void clearAudioWaveform();
    void clearTonguePlot();
};

#endif // PATIENTDIALOG_H
