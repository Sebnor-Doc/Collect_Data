#include "sensordisplay.h"
#include "ui_sensordisplay.h"

#include <QDateTime>

SensorDisplay::SensorDisplay(ReadSensors *rs, QWidget *parent) : QDialog(parent), ui(new Ui::SensorDisplay)
{
    this->rs = rs;
    rs->beginRecording();

    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);


    // Set graphs
    ui->magPlot->plotLayout()->clear();

    for (int i = 0; i < NUM_OF_SENSORS; i++) {

        int row = i / NUM_OF_SENSORS_PER_BOARD;
        int col = i % NUM_OF_SENSORS_PER_BOARD;

        plots[i].axis = new QCPAxisRect(ui->magPlot);
        plots[i].axis->setupFullAxesBox(true);
        plots[i].axis->axis(QCPAxis::atLeft)->setRange(-RANGE_VALS, RANGE_VALS);
        plots[i].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(0, 0, 255));
        plots[i].axis->axis(QCPAxis::atTop)->setLabel(QString("PCB %1 ID %2").arg(row+1).arg(col+1));
        ui->magPlot->plotLayout()->addElement(row, col, plots[i].axis);

        plots[i].graphX = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));
        plots[i].graphY = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));
        plots[i].graphZ = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));

        plots[i].graphX->setPen(QPen(Qt::blue));
        plots[i].graphY->setPen(QPen(Qt::red));
        plots[i].graphZ->setPen(QPen(Qt::green));

    }

    // Start Timer
    magTimer = new QTimer(this);
    magTimer->start(50);
    startTime = -1;

    // Manage connection
    connect(magTimer, SIGNAL(timeout()), this, SLOT(updateMagPlot()));
}

void SensorDisplay::updateMagPlot() {

    // Set time stamp (in sec.)
    double time;

    if (startTime == -1) {
        startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        time = 0.0;
    }
    else {
        time = (QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime) / 1000.0;
    }


    MagData data = rs->getLastPacket();

   for (int i = 0; i < NUM_OF_SENSORS; i++)
   {
       int x = data.at(i*3);
       int y = data.at(i*3 + 1);
       int z = data.at(i*3 + 2);

       // add data to lines:
       plots[i].graphX->addData(time, x);
       plots[i].graphY->addData(time, y);
       plots[i].graphZ->addData(time, z);

       // remove data of lines that's outside visible range:
       double lowerBound = time - 8;
       if (lowerBound > 0) {
           plots[i].graphX->removeDataBefore(lowerBound);
           plots[i].graphY->removeDataBefore(lowerBound);
           plots[i].graphZ->removeDataBefore(lowerBound);
       }

       // make key axis range scroll with the data (at a constant range size of 8):
       plots[i].axis->axis(QCPAxis::atBottom)->setRange(time+0.25, 8, Qt::AlignRight);
    }
   ui->magPlot->replot();


//        //Check to see if sensor is working correctly
//        plots[i].check2 = plots[i].check1;
//        plots[i].check1 = x;

//        if (plots[i].check2-plots[i].check1 == 0)
//            plots[i].working++;
//        else if(plots[i].working>0)
//            plots[i].working--;

//        if (plots[i].working >SENSOR_WORKING_THRESHOLD)
//        {
//            plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabelColor(QColor(255, 0, 0));
//            if (plots[i].working%20 == 0)
//                plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabel(" ");
//            else if (plots[i].working%10 == 0)
//                plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabel("NOT WORKING");

//        }
//        else
//        {
//            plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabelColor(QColor(0, 0, 255));
//            //plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabel("WORKING CORRECTLY @ " + QString::number(1.0/(key-plots[i].key)));
//            plots[i].wideAxisRect->axis(QCPAxis::atBottom)->setLabel("WORKING CORRECTLY");
//        }

//        plots[i].key = time;

}

void SensorDisplay::closeEvent(QCloseEvent *event) {

    magTimer->stop();
    emit closed();
    event->accept();
}

SensorDisplay::~SensorDisplay()
{
    delete ui;
    delete magTimer;
}
