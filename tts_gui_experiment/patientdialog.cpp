#include "patientdialog.h"
#include "ui_patientdialog.h"
#include <QLabel>

PatientDialog::PatientDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PatientDialog)
{
    ui->setupUi(this);

    // Avoid the other widgets to resize when score plot is hidden
    QSizePolicy sp_retain = ui->scorePlot->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->scorePlot->setSizePolicy(sp_retain);

    ui->bioFeedWidget->setVisible(false);

    // Add labels for reference
    for (int i = 0; i < 8; i++) {
        QLabel *label = new QLabel(this);

        if(i == 0 || i == 1 || i == 3) {
            label->setText("Ref");
        }
        else {
            label->setText("X");
        }

        label->setFont(QFont("Times", 10, QFont::Light));

        ui->play_ref_layout->addWidget(label, 1);
        playRefLabels.append(label);
    }

    setAudioWaveform();
    setTonguePlot();
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
void PatientDialog::updateVideo(QPixmap image)
{
    if (!ui->bioFeedCheckBox->isChecked() && videoMode != REPLAY_REF) {
        return;
    }

    QPixmap scaledImg = image.scaled(ui->videoFeed->width(), ui->videoFeed->height());
    ui->videoFeed->setPixmap(scaledImg);
}

void PatientDialog::updateVideoMode(VideoMode videoMode)
{
    this->videoMode = videoMode;
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

    clearAudioWaveform();
    clearTonguePlot();
}

void PatientDialog::setNextTrial(int nextTrial)
{
    for (int i = 0; i < playRefLabels.size(); i++) {

        if (i == (nextTrial-1)) {
             playRefLabels[i]->setFont(QFont("Times", 18, QFont::Bold));
        }
        else {
            playRefLabels[i]->setFont(QFont("Times", 10, QFont::Light));
        }
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


/* Audio Waveform */
void PatientDialog::setAudioWaveform()
{
    ui->audioPlot->plotLayout()->clear();

    QCPAxisRect *axis = new QCPAxisRect(ui->audioPlot);

    waveform.keysRange.lower = 0.0;
    waveform.keysRange.upper = 10.0;

    waveform.valuesRange.lower = -0.8;
    waveform.valuesRange.upper = 0.8;

    axis->setupFullAxesBox(true);
    axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atTop)->setLabel("Voice Audio Waveform");
    axis->axis(QCPAxis::atBottom)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atBottom)->setLabel("Time (s)");
    axis->axis(QCPAxis::atLeft)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atLeft)->setLabel("Normalized Amplitude");
    axis->axis(QCPAxis::atLeft)->setRange(waveform.valuesRange);
    axis->axis(QCPAxis::atBottom)->setRange(waveform.keysRange);

    waveform.graph      = ui->audioPlot->addGraph(axis->axis(QCPAxis::atBottom), axis->axis(QCPAxis::atLeft));
    waveform.refGraph   = ui->audioPlot->addGraph(axis->axis(QCPAxis::atBottom), axis->axis(QCPAxis::atLeft));

    waveform.graph->setPen(QPen(Qt::red));
    waveform.refGraph->setPen(QPen(Qt::darkGreen));

    ui->audioPlot->plotLayout()->addElement(0,0, axis);
}

void PatientDialog::updateWaveform(AudioSample sample, bool ref)
{
    QVector<double> time = sample.keys().toVector();
    QVector<double> data = sample.values().toVector();

    QCPGraph *graph = ref ? waveform.refGraph :  waveform.graph;
    graph->addData(time, data);
    graph->rescaleKeyAxis(true);

    ui->audioPlot->replot();
}

void PatientDialog::clearAudioWaveform()
{
    // Clear Waveform
    QVector<QCPGraph*> waveGraphs;
    waveGraphs.append(waveform.graph);
    waveGraphs.append(waveform.refGraph);

    foreach (QCPGraph* graph, waveGraphs) {
        graph->clearData();
        graph->keyAxis()->setRange(waveform.keysRange);
    }
}

/*  Tongue Tracking */
void PatientDialog::setTonguePlot()
{
    ui->tonguePlot->plotLayout()->clear();
    ui->tonguePlot->setInteraction(QCP::iRangeDrag, true);
    ui->tonguePlot->setInteraction(QCP::iRangeZoom, true);

    // Set plots
    for (int i = 0; i < 3; i++) {

        // Setup Localization plane axis
        locaTrajPlots[i].axis = new QCPAxisRect(ui->tonguePlot);
        locaTrajPlots[i].axis->setupFullAxesBox(true);

        // Setup Localization vs. Time (dynamic) axis
        locaTimePlots[i].axis = new QCPAxisRect(ui->tonguePlot);
        locaTimePlots[i].axis->setupFullAxesBox(true);

        // Add 2 vertically stacked plots (top = plane, bottom = dynamic)
        ui->tonguePlot->plotLayout()->addElement(0, i, locaTrajPlots[i].axis);
        ui->tonguePlot->plotLayout()->addElement(1, i, locaTimePlots[i].axis);

        // Create and add Localization graphs (trajectory + reference + leading dot)
        locaTrajPlots[i].graph = new QCPCurve(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                              locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        locaTrajPlots[i].refCurve = new QCPCurve(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                                 locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        ui->tonguePlot->addPlottable(locaTrajPlots[i].graph);
        ui->tonguePlot->addPlottable(locaTrajPlots[i].refCurve);

        locaTrajPlots[i].leadDot = ui->tonguePlot->addGraph(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                                            locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        // Create and add Localization vs. Time graph
        locaTimePlots[i].graph = ui->tonguePlot->addGraph(locaTimePlots[i].axis->axis(QCPAxis::atBottom),
                                                          locaTimePlots[i].axis->axis(QCPAxis::atLeft));

        locaTimePlots[i].refGraph = ui->tonguePlot->addGraph(locaTimePlots[i].axis->axis(QCPAxis::atBottom),
                                                             locaTimePlots[i].axis->axis(QCPAxis::atLeft));
    }

    // Set trajectory plots attributes
    locaTrajPlots[0].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    locaTrajPlots[0].graph->setPen(QPen(Qt::red));
    locaTrajPlots[0].refCurve->setPen(QPen(Qt::darkGreen));
    locaTrajPlots[0].leadDot->setPen(QPen(Qt::black));
    locaTrajPlots[0].leadDot->setLineStyle(QCPGraph::lsNone);
    locaTrajPlots[0].leadDot->setScatterStyle(QCPScatterStyle::ssDisc);
    locaTrajPlots[0].axis->axis(QCPAxis::atTop)->setLabel("X-Y");
    locaTrajPlots[0].axis->axis(QCPAxis::atBottom)->setLabel("X");
    locaTrajPlots[0].axis->axis(QCPAxis::atLeft)->setLabel("Y");
    locaTrajPlots[0].axis->axis(QCPAxis::atBottom)->setRange(-5,5);
    locaTrajPlots[0].axis->axis(QCPAxis::atLeft)->setRange(0,10);

    locaTrajPlots[1].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::darkGreen));
    locaTrajPlots[1].graph->setPen(QPen(Qt::red));
    locaTrajPlots[1].refCurve->setPen(QPen(Qt::darkGreen));
    locaTrajPlots[1].leadDot->setPen(QPen(Qt::black));
    locaTrajPlots[1].leadDot->setLineStyle(QCPGraph::lsNone);
    locaTrajPlots[1].leadDot->setScatterStyle(QCPScatterStyle::ssDisc);
    locaTrajPlots[1].axis->axis(QCPAxis::atTop)->setLabel("X-Z");
    locaTrajPlots[1].axis->axis(QCPAxis::atBottom)->setLabel("X");
    locaTrajPlots[1].axis->axis(QCPAxis::atLeft)->setLabel("Z");
    locaTrajPlots[1].axis->axis(QCPAxis::atBottom)->setRange(-5,5);
    locaTrajPlots[1].axis->axis(QCPAxis::atLeft)->setRange(-5,0);

    locaTrajPlots[2].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::red));
    locaTrajPlots[2].graph->setPen(QPen(Qt::red));
    locaTrajPlots[2].refCurve->setPen(QPen(Qt::darkGreen));
    locaTrajPlots[2].leadDot->setPen(QPen(Qt::black));
    locaTrajPlots[2].leadDot->setLineStyle(QCPGraph::lsNone);
    locaTrajPlots[2].leadDot->setScatterStyle(QCPScatterStyle::ssDisc);
    locaTrajPlots[2].axis->axis(QCPAxis::atTop)->setLabel("Y-Z");
    locaTrajPlots[2].axis->axis(QCPAxis::atBottom)->setLabel("Y");
    locaTrajPlots[2].axis->axis(QCPAxis::atLeft)->setLabel("Z");
    locaTrajPlots[2].axis->axis(QCPAxis::atBottom)->setRange(0,10);
    locaTrajPlots[2].axis->axis(QCPAxis::atLeft)->setRange(-5,0);

    // Set Time series attributes
    locaTimePlots[0].timeRange = new QCPRange(0,5);
    locaTimePlots[0].YRange = new QCPRange(-5,5);

    locaTimePlots[0].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    locaTimePlots[0].graph->setPen(QPen(Qt::red));
    locaTimePlots[0].refGraph->setPen(QPen(Qt::darkGreen));
    locaTimePlots[0].axis->axis(QCPAxis::atTop)->setLabel("X (time)");
    locaTimePlots[0].axis->axis(QCPAxis::atBottom)->setLabel("time(s)");
    locaTimePlots[0].axis->axis(QCPAxis::atBottom)->setRange(*locaTimePlots[0].timeRange);
    locaTimePlots[0].axis->axis(QCPAxis::atLeft)->setLabel("X(cm)");
    locaTimePlots[0].axis->axis(QCPAxis::atLeft)->setRange(*locaTimePlots[0].YRange);

    locaTimePlots[1].timeRange = new QCPRange(0,5);
    locaTimePlots[1].YRange = new QCPRange(0,10);
    locaTimePlots[1].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    locaTimePlots[1].graph->setPen(QPen(Qt::red));
    locaTimePlots[1].refGraph->setPen(QPen(Qt::darkGreen));
    locaTimePlots[1].axis->axis(QCPAxis::atTop)->setLabel("Y (time)");
    locaTimePlots[1].axis->axis(QCPAxis::atBottom)->setLabel("time(s)");
    locaTimePlots[1].axis->axis(QCPAxis::atBottom)->setRange(*locaTimePlots[1].timeRange);
    locaTimePlots[1].axis->axis(QCPAxis::atLeft)->setLabel("Y(cm)");
    locaTimePlots[1].axis->axis(QCPAxis::atLeft)->setRange(*locaTimePlots[1].YRange);

    locaTimePlots[2].timeRange = new QCPRange(0,5);
    locaTimePlots[2].YRange = new QCPRange(-5,0);
    locaTimePlots[2].axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    locaTimePlots[2].graph->setPen(QPen(Qt::red));
    locaTimePlots[2].refGraph->setPen(QPen(Qt::darkGreen));
    locaTimePlots[2].axis->axis(QCPAxis::atTop)->setLabel("Z (time)");
    locaTimePlots[2].axis->axis(QCPAxis::atBottom)->setLabel("time(s)");
    locaTimePlots[2].axis->axis(QCPAxis::atBottom)->setRange(*locaTimePlots[2].timeRange);
    locaTimePlots[2].axis->axis(QCPAxis::atLeft)->setLabel("Z(cm)");
    locaTimePlots[2].axis->axis(QCPAxis::atLeft)->setRange(*locaTimePlots[2].YRange);
}

void PatientDialog::updateTongue(LocaData locaData)
{

    // Convert data
    double timeMs = locaData.time;
    double timeSec = locaData.time / 1000.0;
    double x = locaData.x * 100.0;
    double y = locaData.y * 100.0;
    double z = locaData.z * 100.0;

    // Add point to localization graphs
    locaTrajPlots[0].graph->addData(timeMs, x, y);
    locaTrajPlots[1].graph->addData(timeMs, x, z);
    locaTrajPlots[2].graph->addData(timeMs, y, z);

    // Set leading dots
    locaTrajPlots[0].leadDot->clearData();
    locaTrajPlots[1].leadDot->clearData();
    locaTrajPlots[2].leadDot->clearData();
    locaTrajPlots[0].leadDot->addData(x, y);
    locaTrajPlots[1].leadDot->addData(x, z);
    locaTrajPlots[2].leadDot->addData(y, z);

    // Add point to time series
    locaTimePlots[0].graph->addData(timeSec, x);
    locaTimePlots[1].graph->addData(timeSec, y);
    locaTimePlots[2].graph->addData(timeSec, z);

    // remove data of lines that's outside visible range:
    double lowerBound = timeSec - 4;

    for (int i = 0; i < 3; i++) {

        if (lowerBound > 0) {
            locaTimePlots[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, timeSec+1);
        }
    }

    ui->tonguePlot->replot();
}

void PatientDialog::updateRefTongue(QVector<LocaData> refLocaData)
{
    // Add point to localization graphs
    foreach (LocaData data, refLocaData ) {

        locaTrajPlots[0].refCurve->addData(data.time, data.x, data.y);
        locaTrajPlots[1].refCurve->addData(data.time, data.x, data.z);
        locaTrajPlots[2].refCurve->addData(data.time, data.y, data.z);

        locaTimePlots[0].refGraph->addData(data.time, data.x);
        locaTimePlots[1].refGraph->addData(data.time, data.y);
        locaTimePlots[2].refGraph->addData(data.time, data.z);
    }

    ui->tonguePlot->replot();
}

void PatientDialog::clearTonguePlot()
{
    // Clear all trajectory plots data
    for (int i = 0; i < 3; i++) {
        locaTrajPlots[i].graph->clearData();
        locaTrajPlots[i].refCurve->clearData();

        locaTimePlots[i].graph->clearData();
        locaTimePlots[i].refGraph->clearData();
        locaTimePlots[i].axis->axis(QCPAxis::atBottom)->setRange(*locaTimePlots[i].timeRange);
        locaTimePlots[i].axis->axis(QCPAxis::atLeft)->setRange(*locaTimePlots[i].YRange);
    }
}

/*  Cleaning */
PatientDialog::~PatientDialog()
{
    delete ui;
}


void PatientDialog::on_bioFeedCheckBox_toggled(bool checked)
{
    ui->bioFeedWidget->setVisible(checked);
}
