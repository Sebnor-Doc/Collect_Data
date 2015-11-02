#include "sensordisplay.h"
#include "ui_sensordisplay.h"

#include <QDateTime>

SensorDisplay::SensorDisplay(QWidget *parent, ReadSensors *readSensor) :
    QDialog(parent),
    ui(new Ui::SensorDisplay)
{

    this->rs = readSensor;

    ui->setupUi(this);
    ui->magPlot->plotLayout()->clear();

    // Set graphs
    for (int i = 0; i < NUM_OF_SENSORS; i++) {

        int row = i / NUM_OF_SENSORS_PER_BOARD;
        int col = i % NUM_OF_SENSORS_PER_BOARD;

        plots[i].axis = new QCPAxisRect(ui->magPlot);
        plots[i].axis->setupFullAxesBox(true);
        plots[i].axis->axis(QCPAxis::atLeft)->setRange(-RANGE_VALS, RANGE_VALS);
        plots[i].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(0, 0, 255));
        plots[i].axis->axis(QCPAxis::atTop)->setLabel(QString("PCB %1 ID %2").arg(row+1).arg(col+1));
        plots[i].axis->axis(QCPAxis::atBottom)->setAutoTicks(false);
        ui->magPlot->plotLayout()->addElement(row, col, plots[i].axis);

        plots[i].graphX = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));
        plots[i].graphY = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));
        plots[i].graphZ = ui->magPlot->addGraph(plots[i].axis->axis(QCPAxis::atBottom), plots[i].axis->axis(QCPAxis::atLeft));

        plots[i].graphX->setPen(QPen(Qt::blue));
        plots[i].graphY->setPen(QPen(Qt::red));
        plots[i].graphZ->setPen(QPen(Qt::green));

    }

    rs->Play();

    magTimer = new QTimer(this);
    magTimer->start(10);

    connect(magTimer, SIGNAL(timeout()), this, SLOT(updateMagPlot()));
}

void SensorDisplay::updateMagPlot() {

    for (int i = 0; i < NUM_OF_SENSORS; i++)
    {
        int row = i / NUM_OF_SENSORS_PER_BOARD;
        int col = i % NUM_OF_SENSORS_PER_BOARD;

        // x axis data for real time update
        double time = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
        double x = rs->getSensorData(row, col, 0);
        double y = rs->getSensorData(row, col, 1);
        double z = rs->getSensorData(row, col, 2);

        // add data to lines:
        plots[i].graphX->addData(time, x);
        plots[i].graphY->addData(time, y);
        plots[i].graphZ->addData(time, z);

        // remove data of lines that's outside visible range:
        plots[i].graphX->removeDataBefore(time-8);
        plots[i].graphY->removeDataBefore(time-8);
        plots[i].graphZ->removeDataBefore(time-8);

        // make key axis range scroll with the data (at a constant range size of 8):
        plots[i].axis->axis(QCPAxis::atBottom)->setRange(time+0.25, 8, Qt::AlignRight);

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

    ui->magPlot->replot();
}

void SensorDisplay::closeEvent(QCloseEvent *event) {
    emit closed();
    event->accept();
}

SensorDisplay::~SensorDisplay()
{
    delete ui;
}
