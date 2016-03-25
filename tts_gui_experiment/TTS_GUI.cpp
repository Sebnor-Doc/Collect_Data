#include "TTS_GUI.h"
#include "ui_TTS_GUI.h"

#include <QFileInfo>
#include <QSerialPortInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QAudioEncoderSettings>
#include <QCloseEvent>
#include <QThread>
#include <QCameraInfo>
#include <QtXml>


// Constructor
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{   
    // General initialization
    sessionCompleted = false;

    //Display GUI
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);

    loadConfig();

    // Disable buttons to enforce proper config sequence
    ui->startStopTrialButton->setEnabled(false);
    ui->measureEMFButton->setEnabled(false);

    // Set initial status of VFB
    mode = NO_VFB;
    on_vfbActivationCheckBox_toggled(false);

    // Connect to Mojo
    QThread *magThread = new QThread(this);
    rs.moveToThread(magThread);

    connect(magThread, SIGNAL(started()), &rs, SLOT(process()));
    connect(this, SIGNAL(save(bool)), &rs, SLOT(saveMag(bool)));
    connect(this, SIGNAL(subOutPathSig(QString)), &rs, SLOT(setSubFilename(QString)));

    connect(this, SIGNAL(stopRecording()), &rs, SLOT(stop()));
    connect(&rs, SIGNAL(finished()), magThread, SLOT(quit()));
    connect(magThread, SIGNAL(finished()), magThread, SLOT(deleteLater()));

    magThread->start(QThread::HighestPriority);

    // Intialize video
    setVideo();

    // Initialize Audio
    setAudio();

    // Initialize Tongue trajectory display
    setTongueTraj();
}


/* System and Session Configuration  */
void MainWindow::on_configButton_clicked()
{
    // Load calibration data and initialize sensors[] vector
    QString calibFilename = QString("TTS %1 - %2.xml").arg(ui->serialNumBox->currentText()).arg(ui->magnetTypeBox->currentText());
    loadCalibration(calibFilename);

    // Setup procedures
    setupExperiment();

    // Initialize Localization Thread
    QThread *locaThread = new QThread(this);
    loca.init(sensors, magnet);

    connect(locaThread, SIGNAL(started()), &loca, SLOT(start()));
    connect(&rs, SIGNAL(dataToLocalize(MagData)), &loca, SLOT(processMag(MagData)));
    connect(this, SIGNAL(subOutPathSig(QString)), &loca, SLOT(setFilename(QString)));
    connect(&loca, SIGNAL(packetLocalized(LocaData)), this, SLOT(updateTongueTraj(LocaData)));

    locaThread->start();

    // Set state of buttons
    ui->measureEMFButton->setEnabled(true);
    ui->configButton->setEnabled(false);

    if (mode == VFB_SUB) {
        // Set general parameters
        vfbManager.setVfbFilePath(QCoreApplication::applicationDirPath() + "/vfb.xml");
        vfbManager.setSubId( ui->subNbEdit->text().toInt() );
        vfbManager.setNumTrials( numTrials );

        vfbManager.updateXML(0);        // Set vfb.xml file general properties
        vfbManager.startVFBProgram();   // Start VFB.exe program
        ui->playRefButton->setEnabled(true);

        connect(this, SIGNAL(subOutPathSig(QString)), &vfbManager, SLOT(setSubOutPath(QString)));
    }
}

void MainWindow::loadConfig() {
    // The config file should be in the same folder than executable
    QString configFileLoc = QCoreApplication::applicationDirPath() + "/Config.xml";
    QFile configFile(configFileLoc);

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString title = "Config.xml not found";
        QString msg = "Config.xml cannot be found.\nVerify if the file is in same directory than the executable file of this porgram.";
        QMessageBox::critical(this, title, msg);
    }

    // Load the XML content as a DOM tree
    QDomDocument configXml;
    bool configLoaded = configXml.setContent(&configFile);
    configFile.close();

    if (!configLoaded){
        QString title = "Config.xml corrupted";
        QString msg = "Config.xml cannot be loaded.\nIt may probably be corrupted.\nReplace it with a properly formatted one";
        QMessageBox::critical(this, title, msg);
    }

    // Read content from DOM Tree
    QDomElement root = configXml.firstChildElement();

    // Set Serial Number of TTS
    QDomElement serialNumElt = root.firstChildElement("serialNum");
    int serialIdx = serialNumElt.text().toInt();

    if ( (serialIdx > 0) && (serialIdx < ui->serialNumBox->count()) ) {
        ui->serialNumBox->setCurrentIndex(serialIdx);
    }
    else {
        ui->serialNumBox->setCurrentIndex(0);
    }

    // Set magnet type from index
    QDomElement magnetIdxElt = root.firstChildElement("magnet");
    int magnetIdx = magnetIdxElt.text().toInt();

    if ( (magnetIdx > 0) && (magnetIdx < ui->magnetTypeBox->count()) ) {
        ui->magnetTypeBox->setCurrentIndex(magnetIdx);
    }
    else {
        ui->magnetTypeBox->setCurrentIndex(0);
    }

    // Set Experiment file location
    QDomElement experimentElt = root.firstChildElement("experiment");
    QString experimentFile = experimentElt.text() + ".txt";
    QString experimentPath = QCoreApplication::applicationDirPath() + "/Experiment/" + experimentFile;
    ui->expFileEdit->setPlainText(experimentPath);

    // Set others
    ui->subPathEdit->setPlainText(root.firstChildElement("data").text());
    ui->refPathEdit->setPlainText(root.firstChildElement("referenceRoot").text());
    ui->subNbEdit->setText("1");
}

void MainWindow::loadCalibration(QString calibFilename) {

    QString calibPath = QCoreApplication::applicationDirPath() + "/Calibration/" + calibFilename;
    QFile calibFile(calibPath);

    if (!calibFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString title = "Calibration file not found";
        QString msg = calibPath + " cannot be found.\nVerify the selected TTS serial number.";
        QMessageBox::critical(this, title, msg);
    }

    // Load the XML content as a DOM tree
    QDomDocument calibXml;
    bool calibLoaded = calibXml.setContent(&calibFile);
    calibFile.close();

    if (!calibLoaded){
        QString title = "Calibration file corrupted";
        QString msg = calibPath + " cannot be loaded.\nIt may probably be corrupted.\nReplace it with a properly formatted one";
        QMessageBox::critical(this, title, msg);
    }

    // Read content from DOM Tree
    QDomElement root = calibXml.firstChildElement();

    // Set Magnet properties
    QDomElement magnetElt = root.firstChildElement("magnet");
    double diameter(magnetElt.firstChildElement("diameter").text().toDouble());
    double length(magnetElt.firstChildElement("length").text().toDouble());
    double Br(magnetElt.firstChildElement("Br").text().toDouble());
    double Bt(magnetElt.firstChildElement("Bt").text().toDouble());
    magnet.setProprieties(diameter, length, Bt, Br);


    // Instantiate Sensor objects and set their instance variables (id, position , gain, ...)
    QDomNodeList sensorNodes = root.elementsByTagName("sensor");
    sensors.reserve(sensorNodes.size());

    for (int i = 0; i < sensorNodes.size(); i++)
    {
        QDomElement sensorElt = sensorNodes.at(i).toElement();

        unsigned short  id      = sensorElt.attribute("id").toUShort();
        QVector<double> pos(parseVector(sensorElt.firstChildElement("position").text(), false));
        QVector<double> offset(parseVector(sensorElt.firstChildElement("offset").text(), false));
        QVector<double> gain(parseVector(sensorElt.firstChildElement("gain").text(), true));
        QVector<double> angles(parseVector(sensorElt.firstChildElement("angles").text(), false));

        sensors.push_back(new Sensor(id, pos, angles, gain, offset));
    }

}

void MainWindow::setupExperiment()
{
    // Set utter, utterClass and numTrials vectors
    loadExperimentFile(ui->expFileEdit->toPlainText());

    // Create folder structure to house experimental data
    QString folderPrefix = (mode == VFB_REF) ? "Ref" : "Sub";

    experiment_root = ui->subPathEdit->toPlainText() + "/" + folderPrefix + ui->subNbEdit->text();

    if (!QDir().mkdir(experiment_root)) {
        qDebug() << "Subject root folder already exists: " << experiment_root;
    }

    for (int i = 0; i < classUtter.size(); i++)
    {
        QString classWord = classUtter.at(i);
        classWord.truncate(40);
        classWord = classWord.trimmed();

        QString classPath = experiment_root + "/" + classWord;

        if (!QDir().mkdir(classPath)) {
            qDebug() << "Class path couldn't be created: " << classPath;
        }

        for (int j = 0; j < utter.at(i)->size(); j++)
        {
            QString word = parseUtter(utter.at(i)->at(j));
            QString utterPath = classPath + "/" + word;

            if (!QDir().mkdir(utterPath)){
                qDebug() << "Utterance path couldn't be created: " << utterPath;
            }

            for (int k = 0; k < numTrials; k++)
            {
                QString trialPath = utterPath + "/" + word + "_" + QString::number(k+1);

                if (!QDir().mkdir(trialPath)) {
                    qDebug() << "Trial path couldn't be created: " << trialPath;
                }
            }
        }
    }

    // Populate dropdown menu with first set of experimental words
    for (int i = 0; i < classUtter.size(); i++)
    {
        // Populate class name
        ui->classBox->addItem(classUtter.at(i) + "\t\t" + QString::number(i+1) + "/" + QString::number(classUtter.size()));
    }
    ui->classBox->setCurrentIndex(0);   // Update utterance box due to classBox_currentIdxChanged slot

    for (int trial = 1; trial <= numTrials; trial++) {
        // Populate trial numbers
        ui->trialBox->addItem(QString::number(trial) + " / " + QString::number(numTrials));
    }

    if (mode == VFB_SUB) {
        vfbManager.setRefRootPath(ui->refPathEdit->toPlainText());
    }

    // Set instance variables for folder/file paths
    setFilePath();
}


/**
 * Set utter, classUtter and numTrials vectors from a list of utterances in the experiment file
 * @param experimentFile File path of list of utterances
 */
void MainWindow::loadExperimentFile(QString experimentFile)
{
    QFile input(experimentFile);
    QTextStream in(&input);
    in.setCodec("UTF-8");

    if (input.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!in.atEnd())
        {
            QString line = in.readLine();

            if (line.trimmed().isEmpty() || line.trimmed().at(0) == '%')
            {
                continue;
            }

            else if (line.contains("CLASS", Qt::CaseInsensitive))
            {
                // Add class of utterance to classUtter
                int ind = line.indexOf(':');
                classUtter.push_back((line.mid(ind+1)).trimmed());

                // Initialize new list of utterances
                utter.push_back(new QStringList());
            }

            else if (line.contains("Iter", Qt::CaseInsensitive))
            {
                int ind = line.indexOf(':');
                numTrials = line.mid(ind+1).trimmed().toInt();
            }

            else
            {
                utter.back()->append(line.trimmed());
            }
        }
    }
    else {
        qDebug() << "Cannot open experiment file located at:\n" << experimentFile << endl;
    }
}


/* Measure EMF */
void MainWindow::on_measureEMFButton_clicked()
{
    // Set EMF file path
    QString currentDateTime = QDateTime::currentDateTime().toString("yy-MM-dd_hh-mm");
    emfFile = experiment_root + "/EMF_" + currentDateTime + ".txt";

    // Start saving mag data to EMF file
    rs.setFilename(emfFile);
    rs.setEmf(true);
    rs.saveMag(true);

    // Record mag data for 1 second
    QTimer::singleShot(1000, this, SLOT(saveEMF()));
}

void MainWindow::saveEMF() {

    // Stop saving and recording magnetic data
    rs.saveMag(false);
    rs.setEmf(false);

    QVector<int> avgEMF(3*NUM_OF_SENSORS);
    avgEMF.fill(0);

    int numSamples = 0;

    // Compute average of EMF samples
    QFile rawEmfFile(emfFile);
    QTextStream emfDataStream(&rawEmfFile);

    if (rawEmfFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while(!emfDataStream.atEnd()) {
            QStringList sample = emfDataStream.readLine().split(" ");
            numSamples++;

            for (int i = 0; i < avgEMF.size(); i++) {
                avgEMF[i] += ((QString)sample.at(i)).toInt();
            }
        }

    }
    else {
        qDebug() << "ERROR: CANNOT READ EMF FILE";
    }
    rawEmfFile.close();

    // Replace current EMF file content with only the EMF average
    QFile avgEmfFile(emfFile);
    QTextStream avgEmfStream(&avgEmfFile);

    if (avgEmfFile.open(QIODevice::WriteOnly | QIODevice::Text)) {

        for (int i = 0; i < NUM_OF_SENSORS; i++) {
            int x = avgEMF.at(3*i) / numSamples;
            int y = avgEMF.at(3*i + 1) / numSamples;
            int z = avgEMF.at(3*i + 2) / numSamples;
            avgEmfStream << x << " " << y << " " << z << endl;

            QVector<double> emfSensor(3);
            emfSensor[0] = x; emfSensor[1] = y; emfSensor[2] = z;
            sensors[i]->setEMF(emfSensor);
        }

    }
    else {
        qDebug() << "ERROR: CANNOT WRITE AVERAGE EMF TO FILE";
    }
    avgEmfFile.close();

    // Update buttons status
    ui->measureEMFButton->setText("EMF Measured!");
    ui->measureEMFButton->setEnabled(false);
    ui->startStopTrialButton->setEnabled(true);
}


/* Data collection */
void MainWindow::on_startStopTrialButton_toggled(bool checked)
{
    if (checked) {
        ui->startStopTrialButton->setText("Stop");
        beginTrial();

    } else {        
        stopTrial();
    }
}

void MainWindow::beginTrial(){
    // Update output file paths
    setFilePath();
    emit subOutPathSig(subOutPath);

    //Color the boxes
    QString formatUtterance = QString("<font size=\"34\" color=\"red\">%1</font>")
                              .arg(utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex()));
    ui->utteranceBrowser->setText(formatUtterance);

    // Disable video playback
    if (ui->videoPlaybackRadio->isChecked()) {
        ui->videoPlaybackRadio->setChecked(false);
        ui->videoShowRadio->setChecked(true);
        videoManager();
    }

    ui->videoPlaybackRadio->setEnabled(false);

    // Send signal to start saving
    emit save(true);

    // Reset plots
    clearPlots();

    if (mode == VFB_SUB) {
        vfbManager.updateXML(1);        // Update Visual Feedback XML file
    }
}

void MainWindow::stopTrial(){

    emit save(false);

    // Update next trial
    sessionCompleted = false;
    int classIdx = ui->classBox->currentIndex();
    int utterIdx = ui->utteranceBox->currentIndex();
    int trialIdx = ui->trialBox->currentIndex();

    int numUtter =  ((QStringList*) utter.at(classIdx))->size();
    int numClass = classUtter.size();

    if ((utterIdx + 1) < numUtter) {
        // Update to next utterance of current class
        ui->utteranceBox->setCurrentIndex(utterIdx + 1);
    }
    else if ((classIdx + 1) < numClass) {
        // Update to next class
        ui->classBox->setCurrentIndex(classIdx + 1);
    }
    else if ((trialIdx + 1) < numTrials) {
        // Update to next trial
        ui->trialBox->setCurrentIndex(trialIdx + 1);
    }
    else {
        // Session is over, all utterances haven been recorded
        sessionCompleted = true;
        ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(classIdx)->at(utterIdx) + QString("</font>"));
    }

    // Update start/stop button status based on session completion status
    if (sessionCompleted) {
        ui->startStopTrialButton->setText("Done!");
        ui->startStopTrialButton->setEnabled(false);
    }
    else {
        ui->startStopTrialButton->setText("Start");
    }

    // Manage video playback
    ui->videoPlaybackRadio->setEnabled(true);
}


/* Graph Sensor data */
void MainWindow::on_showMagButton_toggled(bool checked)
{
    if (checked) {
        ui->showMagButton->setText("Hide Sensors");
        sensorUi = new SensorDisplay(&rs, this);
        sensorUi->show();
        connect(sensorUi, SIGNAL(closed()), this, SLOT(sensorDisplayClosed()));

    } else {
        ui->showMagButton->setText("Show Sensors");
        disconnect(sensorUi, SIGNAL(closed()), this, SLOT(sensorDisplayClosed()));

        if (sensorUi) {
            delete sensorUi;
        }

    }
}

void MainWindow::sensorDisplayClosed(){
    ui->showMagButton->setChecked(false);
}


/* Video */
void MainWindow::setVideo() {

    // Initialize a thread
    QThread *videoThread = new QThread(this);
    video.moveToThread(videoThread);

    // Manage the thread lifecycle
    connect(videoThread, SIGNAL(started()), &video, SLOT(process()));
    connect(this, SIGNAL(stopRecording()), &video, SLOT(stop()));
    connect(&video, SIGNAL(finished()), videoThread, SLOT(quit()));
    connect(videoThread, SIGNAL(finished()), videoThread, SLOT(deleteLater()));

    // Update video feed with image provided by video thread
    connect(&video, SIGNAL(processedImage(QPixmap)), ui->videoFeed, SLOT(setPixmap(QPixmap)));

    // Manage saving status during data collection
    connect(this, SIGNAL(save(bool)), &video, SLOT(saveVideo(bool)));
    connect(this, SIGNAL(subOutPathSig(QString)), &video, SLOT(setFilename(QString)));

    // Select a radio button calls videoManager to set the display mode
    connect(ui->videoShowRadio, SIGNAL(toggled(bool)), this, SLOT(videoManager()));
    connect(ui->videoPlaybackRadio, SIGNAL(toggled(bool)), this, SLOT(videoManager()));
    connect(ui->videoHideRadio, SIGNAL(toggled(bool)), this, SLOT(videoManager()));
    connect(this, SIGNAL(videoMode(VideoMode)), &video, SLOT(displayVideo(VideoMode)));

    // Manage playback
    connect(&video, SIGNAL(replayFrameRange(int, int)), ui->videoSlider, SLOT(setRange(int, int)));
    connect(ui->videoSlider, SIGNAL(valueChanged(int)), &video, SLOT(updatePlaybackIdx(int)));

    ui->videoPlaybackRadio->setEnabled(false);
    ui->videoSlider->setEnabled(false);

    // Set videoMode to view camera feed
    videoManager();

    // Start video data acquisition
    videoThread->start();
}

void MainWindow::videoManager() {

    // Reset slider position to 0 and disable
    ui->videoSlider->setValue(0);
    ui->videoSlider->setEnabled(false);

    // Set video display mode
    // 0 = Live feed ; 1 = Playback ; Other = No feed
    VideoMode mode;

    if (ui->videoShowRadio->isChecked()) {
        mode = LIVE_FEED;
    }

    else if (ui->videoPlaybackRadio->isChecked()) {
        ui->videoSlider->setEnabled(true);
        mode = REPLAY_SUB;
    }

    else {
        mode = NO_FEED;
    }

    emit videoMode(mode);
}


/* Audio */
void MainWindow::setAudio(){

    // Set Waveform
    setWaveform();

    // Set Thread
    QThread *audioThread = new QThread(this);
    voice.moveToThread(audioThread);

    connect(audioThread, SIGNAL(started()), &voice, SLOT(init()));
    connect(&voice, SIGNAL(audioSample(AudioSample, bool)), this, SLOT(updateWaveform(AudioSample, bool)));

    // Manage saving status during data collection
    connect(this, SIGNAL(save(bool)), &voice, SLOT(save(bool)));
    connect(this, SIGNAL(subOutPathSig(QString)), &voice, SLOT(setFilename(QString)));
    connect(this, SIGNAL(stopRecording()), &voice, SLOT(stop()));

    audioThread->start();
}

void MainWindow::setWaveform(){
    ui->waveformQCP->plotLayout()->clear();

    QCPAxisRect *axis = new QCPAxisRect(ui->waveformQCP);

    waveform.keysRange.lower = 0.0;
    waveform.keysRange.upper = 10.0;

    waveform.valuesRange.lower = -0.5;
    waveform.valuesRange.upper = 0.5;

    axis->setupFullAxesBox(true);
    axis->axis(QCPAxis::atTop)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atTop)->setLabel("Voice Audio Waveform");
    axis->axis(QCPAxis::atBottom)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atBottom)->setLabel("Time (s)");
    axis->axis(QCPAxis::atLeft)->setLabelColor(QColor(Qt::blue));
    axis->axis(QCPAxis::atLeft)->setLabel("Normalized Amplitude");
    axis->axis(QCPAxis::atLeft)->setRange(waveform.valuesRange);
    axis->axis(QCPAxis::atBottom)->setRange(waveform.keysRange);

    waveform.graph      = ui->waveformQCP->addGraph(axis->axis(QCPAxis::atBottom), axis->axis(QCPAxis::atLeft));
    waveform.refGraph   = ui->waveformQCP->addGraph(axis->axis(QCPAxis::atBottom), axis->axis(QCPAxis::atLeft));

    waveform.refGraph->setPen(QPen(Qt::red));
    waveform.refGraph->setPen(QPen(Qt::darkGreen));

    ui->waveformQCP->plotLayout()->addElement(0,0, axis);
}

void MainWindow::updateWaveform(AudioSample sample, bool ref) {

    QVector<double> time = sample.keys().toVector();
    QVector<double> data = sample.values().toVector();


    QCPGraph *graph = ref ? waveform.refGraph :  waveform.graph;
    graph->addData(time, data);
    graph->rescaleKeyAxis(true);

    ui->waveformQCP->replot();
}


/* Tongue Trajectory */
void MainWindow::setTongueTraj()
{
    ui->locaPlot->plotLayout()->clear();
    ui->locaPlot->setInteraction(QCP::iRangeDrag, true);
    ui->locaPlot->setInteraction(QCP::iRangeZoom, true);

    // Set plots
    for (int i = 0; i < 3; i++) {

        // Setup Localization plane axis
        locaTrajPlots[i].axis = new QCPAxisRect(ui->locaPlot);
        locaTrajPlots[i].axis->setupFullAxesBox(true);

        // Setup Localization vs. Time (dynamic) axis
        locaTimePlots[i].axis = new QCPAxisRect(ui->locaPlot);
        locaTimePlots[i].axis->setupFullAxesBox(true);

        // Add 2 vertically stacked plots (top = plane, bottom = dynamic)
        ui->locaPlot->plotLayout()->addElement(0, i, locaTrajPlots[i].axis);
        ui->locaPlot->plotLayout()->addElement(1, i, locaTimePlots[i].axis);

        // Create and add Localization graphs (trajectory + reference + leading dot)
        locaTrajPlots[i].graph = new QCPCurve(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                          locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        locaTrajPlots[i].refCurve = new QCPCurve(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                              locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        ui->locaPlot->addPlottable(locaTrajPlots[i].graph);
        ui->locaPlot->addPlottable(locaTrajPlots[i].refCurve);

        locaTrajPlots[i].leadDot = ui->locaPlot->addGraph(locaTrajPlots[i].axis->axis(QCPAxis::atBottom),
                                                    locaTrajPlots[i].axis->axis(QCPAxis::atLeft));

        // Create and add Localization vs. Time graph
        locaTimePlots[i].graph = ui->locaPlot->addGraph(locaTimePlots[i].axis->axis(QCPAxis::atBottom),
                                                    locaTimePlots[i].axis->axis(QCPAxis::atLeft));

        locaTimePlots[i].refGraph = ui->locaPlot->addGraph(locaTimePlots[i].axis->axis(QCPAxis::atBottom),
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

void MainWindow::updateTongueTraj(LocaData locaData){

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
//    double lowerBound = timeSec - 10;
    double lowerBound = timeSec - 4;

    for (int i = 0; i < 3; i++) {
        if (lowerBound > 0) {
//            locaTimePlots[i].graph->removeDataBefore(lowerBound);
            locaTimePlots[i].axis->axis(QCPAxis::atBottom)->setRange(lowerBound, timeSec+1);
        }

        // make key axis range scroll with the data (at a constant range size of 8):
//        locaTimePlots[i].axis->axis(QCPAxis::atBottom)->setRange(timeSec, 10, Qt::AlignRight);

    }

    ui->locaPlot->replot();
}

void MainWindow::updateRefTongueTraj()
{
    QVector<LocaData> refLocaData = vfbManager.getRefLocaData();

    // Add point to localization graphs
    foreach (LocaData data, refLocaData ) {

        locaTrajPlots[0].refCurve->addData(data.time, data.x, data.y);
        locaTrajPlots[1].refCurve->addData(data.time, data.x, data.z);
        locaTrajPlots[2].refCurve->addData(data.time, data.y, data.z);

        locaTimePlots[0].refGraph->addData(data.time, data.x);
        locaTimePlots[1].refGraph->addData(data.time, data.y);
        locaTimePlots[2].refGraph->addData(data.time, data.z);
    }

    // resize graph
    ui->locaPlot->replot();
}

void MainWindow::clearPlots() {

    // Clear all trajectory plots data
    for (int i = 0; i < 3; i++) {
        locaTrajPlots[i].graph->clearData();
        locaTrajPlots[i].refCurve->clearData();

        locaTimePlots[i].graph->clearData();
        locaTimePlots[i].refGraph->clearData();
        locaTimePlots[i].axis->axis(QCPAxis::atBottom)->setRange(*locaTimePlots[i].timeRange);
        locaTimePlots[i].axis->axis(QCPAxis::atLeft)->setRange(*locaTimePlots[i].YRange);
    }

    // Clear Waveform
    QVector<QCPGraph*> waveGraphs;
    waveGraphs.append(waveform.graph);
    waveGraphs.append(waveform.refGraph);

    foreach (QCPGraph* graph, waveGraphs) {
        graph->clearData();
        graph->keyAxis()->setRange(waveform.keysRange);
    }
}

/* Manage Drop-down lists */
void MainWindow::on_trialBox_currentIndexChanged(int index)
{
    // Reset class box
    ui->classBox->setCurrentIndex(0);
    on_classBox_currentIndexChanged(0);
}

void MainWindow::on_classBox_currentIndexChanged(int index)
{
    disconnect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));

    // Clear utterance and trial drop-down list
    ui->utteranceBox->clear();

    // Populate list of utterances for this class
    for(int j = 0; j < utter.at(index)->size(); j++)
    {
        QString currentUtter = parseUtter(utter.at(index)->at(j));
        QString formatUtter = currentUtter + "\t\t" + QString::number(j+1) + "/" + QString::number(utter.at(index)->size());
        ui->utteranceBox->addItem(formatUtter);
    }

    connect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));

    // Reset drop-down list to first item
    ui->utteranceBox->setCurrentIndex(0);
    on_utteranceBox_currentIndexChanged(0);
}

void MainWindow::on_utteranceBox_currentIndexChanged(int index)
{
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(index) + QString("</font>"));

    // Enable Start/Stop button if session was completed
    if (sessionCompleted) {
        ui->startStopTrialButton->setEnabled(true);
        ui->startStopTrialButton->setText("Start");
    }
}


/* ********************************************************* *
 *              Helper Methods                               *
 * ********************************************************* */
void MainWindow::setFilePath()
{
    experiment_class = classUtter.at(ui->classBox->currentIndex());
    experiment_class.truncate(40);
    experiment_class = experiment_class.trimmed();

    experiment_utter = parseUtter(utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex()));

    int trial = ui->trialBox->currentIndex() + 1;

    subOutPath = experiment_root + "/" + experiment_class + "/" + experiment_utter + "/" +
            experiment_utter + "_" + QString::number(trial) + "/" + experiment_utter + "_" +
            QString::number(trial);

    if (mode == VFB_SUB) {
        QString relativeRefOutPath = QString("%1/%2/%2_1/%2_1").arg(experiment_class).arg(experiment_utter);
        vfbManager.setRefOutPath(relativeRefOutPath);
        vfbManager.setCurrentUtter(ui->utteranceBrowser->toPlainText());
        vfbManager.setCurrentTrial(trial);
    }
}

QVector<double> MainWindow::parseVector(QString myString, bool matrix)
{
    QVector<double> output;

    myString.remove("[");
    myString.remove("]");
    QStringList list = myString.split(";");

    for (int i = 0; i < list.size(); i++) {

        QString elt = list.at(i);

        if (matrix) {
            QStringList row = elt.split(" ");
            elt = row.at(i);
        }

        output.push_back(elt.toDouble());
    }

    return output;
}

QString MainWindow::parseUtter(QString rawUtter) {

    QString parsedUtter;

    // For Second Language Learner protocol,
    // Keep only the first part (english phonetic representation)
    // if the utter contains translation and original language
    if (rawUtter.contains(";")) {
        int idxSemiCol = rawUtter.indexOf(";");
        parsedUtter = (idxSemiCol != 0) ? rawUtter.left(idxSemiCol) : "";
    }
    else {
        parsedUtter = rawUtter;
    }

    // Remove special characters that cannot be used as folder/file name
    QStringList specialChar;
    specialChar << "?" << ":" << "/" << "\\" << "*" << "<" << ">" << "|";

    for (QString charac : specialChar) {
        parsedUtter.remove(charac);
    }

    // Format the utterance by removing empty spaces and reduce length
    parsedUtter.truncate(40);
    parsedUtter = parsedUtter.trimmed();

    return parsedUtter;
}

/* Closing methods */
void MainWindow::closeEvent(QCloseEvent *event){

    emit stopRecording();

    // Closing procedures
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_vfbActivationCheckBox_toggled(bool checked)
{
    ui->vfbRefModeRadio->setEnabled(checked);
    ui->vfbSubModeRadio->setEnabled(checked);

    ui->vfbSubModeRadio->setChecked(checked);
    on_vfbSubModeRadio_toggled(checked);

    if(!checked) {
        mode = NO_VFB;
    }
}

void MainWindow::on_vfbSubModeRadio_toggled(bool checked)
{
    ui->refPathEdit->setEnabled(checked);
    ui->refWidget->setVisible(checked);

    ui->playRefButton->setVisible(checked);
    ui->playRefButton->setEnabled(false);

    mode = checked ? VFB_SUB : VFB_REF;
}

void MainWindow::on_playRefButton_clicked()
{
    setFilePath();
//    emit subOutPathSig(vfbManager.getRefOutPath());

    vfbManager.playAudio();

    // Update Tongue trajectory with reference
    clearPlots();
    updateRefTongueTraj();

    // Update audio waveform
    vfbManager.getAudioSample();
    connect(&vfbManager, SIGNAL(audioSample(AudioSample, bool)), this, SLOT(updateWaveform(AudioSample, bool)));

    // Update video feed
    video.setFilename(vfbManager.getRefOutPath());
    emit videoMode(REPLAY_REF);
}
