#ifndef SENSORDISPLAY_H
#define SENSORDISPLAY_H

#define WIN32_LEAN_AND_MEAN // Necessary due to problems with Boost
#include <QDialog>
#include "qcustomplot.h"
#include "common.h"
#include "ReadSensors.h"
#include <QTimer>

namespace Ui {
class SensorDisplay;
}

class SensorDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit SensorDisplay(ReadSensors *rs, QWidget *parent = 0);
    ~SensorDisplay();

private:
    Ui::SensorDisplay *ui;
    ReadSensors *rs;
    QTimer *magTimer;
    qint64 startTime;

    typedef struct Plot {
        QCPGraph *graphX;
        QCPGraph *graphY;
        QCPGraph *graphZ;
        QCPGraph *graphXDot;
        QCPGraph *graphYDot;
        QCPGraph *graphZDot;
        QCPAxisRect *axis;
        int check1, check2;
        int working;
        double key;
        QCPCurve *parCurve;
    } Plot;
    Plot plots[NUM_OF_SENSORS];


public slots:
    void updateMagPlot(MagData magData);
    void closeEvent(QCloseEvent *event);

signals:
    void closed();

};

#endif // SENSORDISPLAY_H
