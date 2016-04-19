#include "patientdialog.h"
#include "ui_patientdialog.h"

PatientDialog::PatientDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PatientDialog)
{
    ui->setupUi(this);

    // Avoid the other widgets to resize when score plot is hidden
    QSizePolicy sp_retain = ui->scorePlot->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->scorePlot->setSizePolicy(sp_retain);
}

void PatientDialog::updateUtter(QString utter)
{
    QStringList utterList = utter.split("\n");

    if (utterList.size() == 3) {
        ui->utterEdit->setPlainText(utterList.at(0));
        ui->koreanEdit->setPlainText(utterList.at(1));
        ui->englishEdit->setPlainText(utterList.at(2));
    }

    else {
        ui->utterEdit->setPlainText(utter);
    }
}

void PatientDialog::updateVideo(QPixmap image)
{
    QPixmap scaledImg = image.scaled(ui->videoFeed->width(), ui->videoFeed->height());
    ui->videoFeed->setPixmap(scaledImg);
}

void PatientDialog::recording(bool isRecording)
{
    QString styleSheet;

    if (isRecording) {
        styleSheet = "color: red";
    }
    else {
        styleSheet = "color: grey";
    }

    ui->utterEdit->setStyleSheet(styleSheet);
}


/* Visual Feedback as Scores */
void PatientDialog::updateScores(Scores scores)
{
    double scoreArray[5] = {scores.loca, scores.mag, scores.voice, scores.lips, scores.avg};
    QRgb colors[5];
    colorGrad.colorize(scoreArray, QCPRange(0.0, 10.0), colors, 5);

    locaBars[currentTrial]->removeData(currentTrial);
    locaBars[currentTrial]->addData(currentTrial, scores.loca);
    locaBars[currentTrial]->setBrush(QBrush(QColor(colors[0])));

    magBars[currentTrial]->removeData(currentTrial);
    magBars[currentTrial]->addData(currentTrial, scores.mag);
    magBars[currentTrial]->setBrush(QBrush(QColor(colors[1])));

    voiceBars[currentTrial]->removeData(currentTrial);
    voiceBars[currentTrial]->addData(currentTrial, scores.voice);
    voiceBars[currentTrial]->setBrush(QBrush(QColor(colors[2])));

    lipsBars[currentTrial]->removeData(currentTrial);
    lipsBars[currentTrial]->addData(currentTrial, scores.lips);
    lipsBars[currentTrial]->setBrush(QBrush(QColor(colors[3])));

    avgScoreBars[currentTrial]->removeData(currentTrial);
    avgScoreBars[currentTrial]->addData(currentTrial, scores.avg);
    avgScoreBars[currentTrial]->setBrush(QBrush(QColor(colors[4])));

    ui->scorePlot->replot();

    // Set digital indicator
    ui->avgScoreEdit->setText(QString::number(scores.avg, 'f', 2)); // Decimal with 2 digits after decimal point

    // Set visual cue for next word
    ui->utterEdit->setStyleSheet("color: green");
}

void PatientDialog::setScorePlot(int numTrials) {

    this->numTrials = numTrials;

    ui->scorePlot->plotLayout()->clear();

    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->scorePlot);
    QCPRange barRangeX(0, numTrials + 1);
    QCPRange barRangeY(0, 10);

    QCPAxisRect *locaAxis = new QCPAxisRect(ui->scorePlot);
    QCPAxisRect *magAxis = new QCPAxisRect(ui->scorePlot);
    QCPAxisRect *voiceAxis = new QCPAxisRect(ui->scorePlot);
    QCPAxisRect *lipsAxis = new QCPAxisRect(ui->scorePlot);
    QCPAxisRect *avgScoreAxis = new QCPAxisRect(ui->scorePlot);

    QVector<QCPAxisRect*> scoreAxes;
    scoreAxes << locaAxis << magAxis << voiceAxis << lipsAxis << avgScoreAxis;

    foreach (QCPAxisRect* axis, scoreAxes) {
        axis->setupFullAxesBox(true);
        axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
        axis->axis(QCPAxis::atBottom)->setRange(barRangeX);
        axis->axis(QCPAxis::atBottom)->setAutoTickStep(false);
        axis->axis(QCPAxis::atBottom)->setTickStep(1);
        axis->axis(QCPAxis::atBottom)->setAutoSubTicks(false);
        axis->axis(QCPAxis::atBottom)->setSubTickCount(0);
        axis->axis(QCPAxis::atLeft)->setRange(barRangeY);
        axis->axis(QCPAxis::atLeft)->setAutoTickStep(false);
        axis->axis(QCPAxis::atLeft)->setTickStep(1);
        axis->setMarginGroup(QCP::msLeft, marginGroup);
    }


    locaAxis->axis(QCPAxis::atTop)->setLabel("Localization");
    magAxis->axis(QCPAxis::atTop)->setLabel("Magnetic");
    voiceAxis->axis(QCPAxis::atTop)->setLabel("Voice");
    lipsAxis->axis(QCPAxis::atTop)->setLabel("Lips");
    avgScoreAxis->axis(QCPAxis::atTop)->setLabel("Average");

    ui->scorePlot->plotLayout()->addElement(0, 0, avgScoreAxis);
    ui->scorePlot->plotLayout()->addElement(1, 0, locaAxis);
    ui->scorePlot->plotLayout()->addElement(1, 1, magAxis);
    ui->scorePlot->plotLayout()->addElement(2, 0, voiceAxis);
    ui->scorePlot->plotLayout()->addElement(2, 1, lipsAxis);

    for (int i = 0; i <= numTrials; i++) {
        locaBars.append(new QCPBars(locaAxis->axis(QCPAxis::atBottom), locaAxis->axis(QCPAxis::atLeft)));
        magBars.append(new QCPBars(magAxis->axis(QCPAxis::atBottom), magAxis->axis(QCPAxis::atLeft)));
        voiceBars.append(new QCPBars(voiceAxis->axis(QCPAxis::atBottom), voiceAxis->axis(QCPAxis::atLeft)));
        lipsBars.append(new QCPBars(lipsAxis->axis(QCPAxis::atBottom), lipsAxis->axis(QCPAxis::atLeft)));
        avgScoreBars.append(new QCPBars(avgScoreAxis->axis(QCPAxis::atBottom), avgScoreAxis->axis(QCPAxis::atLeft)));
    }

    QVector<QVector<QCPBars*>> scoreBars;
    scoreBars << locaBars << magBars << voiceBars << lipsBars << avgScoreBars;

    foreach (QVector<QCPBars*> bars, scoreBars) {

        foreach(QCPBars* bar, bars) {
            bar->setWidth(0.2);
            bar->setPen(QPen(QColor(Qt::black)));

            QVector<double> keys(numTrials+2);
            QVector<double> values(keys.size(), 0.0);

            for (int key = 0; key < keys.size(); key++) {
                keys[key] = key;
            }
            bar->setData(keys, values);

            ui->scorePlot->addPlottable(bar);
        }
    }

    // Set Color Gradient and Scale
    colorGrad.loadPreset(QCPColorGradient::gpIon);
    QCPColorScale *colorScale = new QCPColorScale(ui->scorePlot);
    colorScale->setGradient(colorGrad);
    colorScale->setDataRange(barRangeY);
    colorScale->setMarginGroup(QCP::msLeft, marginGroup);
    colorScale->axis()->setAutoTickStep(false);
    colorScale->axis()->setTickStep(1);
    colorScale->axis()->setAutoSubTicks(false);
    colorScale->axis()->setSubTickCount(0);

    ui->scorePlot->plotLayout()->addElement(0, 2, colorScale);
    ui->scorePlot->replot();
}

void PatientDialog::setCurrentTrial(int trial)
{
    currentTrial = trial;

    if (currentTrial == 1) {
        clearScorePlot();
    }
}

void PatientDialog::showScores(bool show)
{
    ui->scorePlot->setVisible(show);
    ui->avgScoreLabel->setVisible(show);
    ui->avgScoreEdit->setVisible(show);
}

void PatientDialog::clearScorePlot()
{
    QVector<QVector<QCPBars*>> scoreBars;
    scoreBars << locaBars << magBars << voiceBars << lipsBars << avgScoreBars;

    foreach (QVector<QCPBars*> bars, scoreBars) {

        foreach(QCPBars* bar, bars) {

            QVector<double> keys(numTrials+2);
            QVector<double> values(keys.size(), 0.0);

            for (int key = 0; key < keys.size(); key++) {
                keys[key] = key;
            }
            bar->setData(keys, values);
        }
    }

    ui->scorePlot->replot();
}


/*  Cleaning */
PatientDialog::~PatientDialog()
{
    delete ui;
}

