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

    // Score plot
    QVector<QCPBars*> locaBars;
    QVector<QCPBars*> magBars;
    QVector<QCPBars*> voiceBars;
    QVector<QCPBars*> lipsBars;
    QVector<QCPBars*> avgScoreBars;
    QCPColorGradient colorGrad;
    int currentTrial;

public:
    void setScorePlot(int numTrials);
    void setCurrentTrial(int trial);

public slots:
    void updateUtter(QString utter);
    void updateScores(Scores scores);
    void updateVideo(QPixmap image);
    void recording(bool isRecording);

private:
    void clearScorePlot();
};

#endif // PATIENTDIALOG_H
