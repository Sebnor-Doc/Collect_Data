#include "ui_TTS_GUI.h"
#include "TTS_GUI.h"
#include <QFileInfo>
#include <QSerialPortInfo>
#include <pugixml.hpp>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include "CImg.h"
#include <QUrl>
#include <QAudioEncoderSettings>
#include <QCloseEvent>

#include <QCameraInfo>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
using boost::escaped_list_separator;
using boost::tokenizer;


// Constructor
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{   
    //Display GUI
    ui->setupUi(this);
    this->setWindowState(Qt::WindowMaximized);

    loadConfig();

    // Disable buttons to enforce proper config sequence
    ui->startStopTrialButton->setEnabled(false);
    ui->measureEMFButton->setEnabled(false);

    // Connect to Mojo
    rs = new ReadSensors(this);
    rs->start(QThread::HighestPriority);

    loca = new Localization(rs, sensors, magnet, this);
    loca->start(QThread::HighPriority);

    // Intialize video
    video = new VideoThread();
    connect(video, SIGNAL(processedImage(QPixmap)), ui->videoFeed, SLOT(setPixmap(QPixmap)) );

    // Initialize Audio
    setAudio();

    // Start video
    video->Play();
    ui->showVideoCheckBox->setChecked(true);
    on_showVideoCheckBox_clicked();
}


/* System and Session Configuration  */
void MainWindow::on_configButton_clicked()
{
    // Load calibration data and initialize sensors[] vector
    QString calibFilename = QString("TTS %1 - %2.xml").arg(ui->serialNumBox->currentText()).arg(ui->magnetTypeBox->currentText());
    loadCalibration(calibFilename);

    // Setup procedures
    setupExperiment();


    loca = new Localization(rs, sensors, magnet, this);

    // Set state of buttons
    ui->measureEMFButton->setEnabled(true);
    ui->configButton->setEnabled(false);
}

void MainWindow::loadConfig() {
    // The config file should be in the same folder than executable
    QString configFile = QCoreApplication::applicationDirPath() + "/Config.xml";

    // Create the root of the tree
    pugi::xml_document config;
    config.load_file(configFile.toStdString().c_str());

    // Create a child node from "config"
    pugi::xml_node nodeConfig = config.child("config");

    // Set system serial number from index
    int serialIdx = boost::lexical_cast<int>(nodeConfig.child("serialNum").child_value());

    if ( (serialIdx > 0) && (serialIdx < ui->serialNumBox->count()) ) {
        ui->serialNumBox->setCurrentIndex(serialIdx);
    }
    else {
        ui->serialNumBox->setCurrentIndex(0);
    }

    // Set magnet type from index
    int magnetIdx = boost::lexical_cast<int>(nodeConfig.child("magnet").child_value());

    if ( (magnetIdx > 0) && (magnetIdx < ui->magnetTypeBox->count()) ) {
        ui->magnetTypeBox->setCurrentIndex(magnetIdx);
    }
    else {
        ui->magnetTypeBox->setCurrentIndex(0);
    }

    // Set Experiment file location
    QString experimentFile = QString::fromStdString(nodeConfig.child("experiment").child_value()) + ".txt";
    QString experimentPath = QCoreApplication::applicationDirPath() + "/Experiment/" + experimentFile;
    ui->expFileEdit->setText(experimentPath);

    // Set others
    ui->subPathEdit->setText(QString::fromStdString(nodeConfig.child("data").child_value()));
    ui->subNbEdit->setText("1");
}

void MainWindow::loadCalibration(QString calibFilename) {

    QString calibPath = QCoreApplication::applicationDirPath() + "/Calibration/" + calibFilename;

    // Create the root of the tree
    pugi::xml_document doc;
    doc.load_file(calibPath.toStdString().c_str());

    // Create a child node from "calibration"
    pugi::xml_node nodeCalibration = doc.child("calibration");

    // Set Magnet's instance variables to data from calibration file
    pugi::xml_node nodeMagnet = nodeCalibration.child("magnet");
    magnet.diameter(boost::lexical_cast<double>(nodeMagnet.child("diameter").child_value()));
    magnet.length(boost::lexical_cast<double>(nodeMagnet.child("length").child_value()));
    magnet.Br(boost::lexical_cast<double>(nodeMagnet.child("Br").child_value()));
    magnet.Bt(boost::lexical_cast<double>(nodeMagnet.child("Bt").child_value()));

    // Instantiate Sensor objects and set their instance variables (id, position , gain, ...)
    int i = 0;
    for (pugi::xml_node nodeSensor = nodeCalibration.child("sensor"); nodeSensor; nodeSensor = nodeSensor.next_sibling("sensor"))
    {
        Sensor *newSensor = new Sensor();
        newSensor->id = nodeSensor.attribute("id").as_uint();
        newSensor->position(loadVector(nodeSensor.child("position").child_value()));
        newSensor->EMF(loadVector(nodeSensor.child("EMF").child_value()));
        newSensor->offset(loadVector(nodeSensor.child("offset").child_value()));
        newSensor->gain(loadMatrix(nodeSensor.child("gain").child_value()));
        newSensor->angles(loadVector(nodeSensor.child("angles").child_value()));

        sensors.push_back(newSensor);
        i++;
    }

}

void MainWindow::setupExperiment()
{
    // Set utter, utterClass and numTrials vectors
    loadExperimentFile(ui->expFileEdit->text());

    // Create folder structure to house experimental data
    experiment_root = ui->subPathEdit->text() + "/Sub" + ui->subNbEdit->text();

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
            QString word = utter.at(i)->at(j);
            word.truncate(40);
            word = word.trimmed();

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


    // Update utterance display
//    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex())
//                                  + QString("</font>"));

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
    rs->setFileLocation(emfFile);

    QString locaFile = experiment_root + "/EMF_Loca.txt";
    loca->setFileLocation(locaFile);

    loca->startSaving();
    rs->startSaving();


    // Record mag data for 1 second
    QTimer::singleShot(1000, this, SLOT(saveEMF()));
}

void MainWindow::saveEMF() {
    // Stop saving and recording magnetic data
    rs->stopSaving();
    loca->stopSaving();

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
            sensors[i]->m_EMF.fill(x, y, z);
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

    //Color the boxes
    QString formatUtterance = QString("<font size=\"34\" color=\"red\">%1</font>")
                              .arg(utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex()));
    ui->utteranceBrowser->setText(formatUtterance);

    // Start data recording
    rs->setFileLocation(experiment_output_path + "_raw_sensor.txt"); // Magnetic stream
    rs->startSaving();

    video->setVideoName(experiment_output_path + "_video.avi");
    video->startSavingVideo();

//    loca->setFileLocation(experiment_output_path + "_localization.txt");
//    loca->Play();

    audio1->stop();
    audio2->stop();
    audio1->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio1.wav"));
    audio2->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio2.wav"));
    audio1->record();
    audio2->record();
}

void MainWindow::stopTrial(){

//    loca->Stop();
    rs->stopSaving();

    video->stopSavingVideo();

    audio1->stop();
    audio2->stop();

    // Update next trial
    bool sessionCompleted = false;
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
}


/* Graph Sensor data */
void MainWindow::on_showMagButton_toggled(bool checked)
{
    if (checked) {
        ui->showMagButton->setText("Hide Sensors");
        sensorUi = new SensorDisplay(rs, this);
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


/* Video player */
void MainWindow::on_showVideoCheckBox_clicked()
{
    if (ui->showVideoCheckBox->isChecked()) {
        video->startEmittingVideo();
    }
    else {
        video->stopEmittingVideo();
    }
}


/* Audio */
void MainWindow::setAudio(){

    audio1 = new QAudioRecorder();
    audio2 = new QAudioRecorder();

    QStringList audioDevices = audio1->audioInputs();

    foreach (QString device, audioDevices) {

        if (device.contains("USB PnP"))
        {
            if (!audio1->audioInput().contains("USB PnP")) {
                audio1->setAudioInput(device);

            } else if (device.compare(audio1->audioInput()) != 0) {
                audio2->setAudioInput(device);
                break;
            }
        }
    }

    // High Settings
    QAudioEncoderSettings settings;
    settings.setQuality(QMultimedia::VeryHighQuality);
    settings.setEncodingMode(QMultimedia::ConstantQualityEncoding);
    settings.setSampleRate(44100);

    audio1->setAudioSettings(settings);
    audio2->setAudioSettings(settings);

    qDebug() << "Audio Source 1: " << audio1->audioInput();
    qDebug() << "Audio Source 2: " << audio2->audioInput() << endl;

    audioProbe1 = new QAudioProbe();
    audioProbe2 = new QAudioProbe();
    audioProbe1->setSource(audio1);
    audioProbe2->setSource(audio2);

    audio1->record();
    audio2->record();

    connect(audioProbe1, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(updateAudioLevels(QAudioBuffer)));
    connect(audioProbe2, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(updateAudioLevels(QAudioBuffer)));

}

void MainWindow::updateAudioLevels(QAudioBuffer audioBuffer) {

//    const quint16 *audioData = audioBuffer.constData<quint16>();
//    qDebug() << "audioData: " << (int) audioData;

//    int numSamples = audioBuffer.frameCount();
//    qDebug() << "Frame count: " << numSamples;


//    qDebug() << "Duration: " << audioBuffer.duration();
//    qDebug() << "Frame count: " << audioBuffer.frameCount();
//    qDebug() << "Start time: " << audioBuffer.startTime();
//    qDebug() << "Sample type: " << audioBuffer.format().sampleType();

//    if (sender() == audioProbe1) {
//        ui->leftVolumeBar->setValue(0);
//    } else {
//        ui->rightVolumeBar->setValue(0);
//    }


}


/* Manage Drop-down lists */
void MainWindow::on_trialBox_currentIndexChanged(int index)
{
    // Reset class box
    ui->classBox->setCurrentIndex(0);
}

void MainWindow::on_classBox_currentIndexChanged(int index)
{
    disconnect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));

    // Clear utterance and trial drop-down list
    ui->utteranceBox->clear();

    // Populate list of utterances for this class
    for(int j = 0; j < utter.at(index)->size(); j++)
    {
        QString formatUtter = utter.at(index)->at(j) + "\t\t" + QString::number(j+1) + "/" + QString::number(utter.at(index)->size());
        ui->utteranceBox->addItem(formatUtter);
    }


    connect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));

    // Reset drop-down list to first item
    ui->utteranceBox->setCurrentIndex(0);
}

void MainWindow::on_utteranceBox_currentIndexChanged(int index)
{
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(index) + QString("</font>"));

    // Enable Start/Stop button if session was completed
    ui->startStopTrialButton->setEnabled(true);
    ui->startStopTrialButton->setText("Start");
}



/* ********************************************************* *
 *              Helper Methods                               *
 * ********************************************************* */
void MainWindow::setFilePath()
{
    experiment_class = classUtter.at(ui->classBox->currentIndex());
    experiment_class.truncate(40);
    experiment_class = experiment_class.trimmed();

    experiment_utter = utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex());
    experiment_utter.truncate(40);
    experiment_utter = experiment_utter.trimmed();

    int trial = ui->trialBox->currentIndex() + 1;

    experiment_output_path = experiment_root + "/" + experiment_class + "/" + experiment_utter + "/" +
            experiment_utter + "_" + QString::number(trial) + "/" + experiment_utter + "_" +
            QString::number(trial);
}

CImg<double> MainWindow::loadVector(string myString)
{
    CImg<double> myVector(1,3);
    boost::escaped_list_separator<char> els("","[;] ","");
    boost::tokenizer< boost::escaped_list_separator<char> > tok(myString, els);
    int i = 0;
    for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it) {
        string value = *it;
        if (value.compare("")) {
            myVector(i++) = boost::lexical_cast<double>(value);
        }
    }

    return myVector;
}

CImg<double> MainWindow::loadMatrix(string myString)
{
    CImg<double> myMatrix(3,3);

    boost::escaped_list_separator<char> els("","[;] ","");
    boost::tokenizer< boost::escaped_list_separator<char> > tok(myString, els);
    int i = 0;
    for(boost::tokenizer< boost::escaped_list_separator<char> >::iterator it = tok.begin(); it != tok.end(); ++it) {
        string value = *it;
        if (value.compare("")) {
            myMatrix(i++) = boost::lexical_cast<double>(value);
        }
    }

    return myMatrix;
}


/* Closing methods */
void MainWindow::closeEvent(QCloseEvent *event){

    // Terminate Read sensor
    rs->stopSaving();
    rs->stopRecording();

    if (video){
        video->Stop();      
    }

//    if (loca) {
//        loca->stopSavingToFile();
//        loca->Stop();
//    }

    // Terminate audio
    if (audio1) {
        audio1->stop();
    }

    if (audio2) {
       audio2->stop();
    }


    // Close Display sensor UI
    if (sensorUi) {
        sensorUi->close();
    }

    // Closing procedures
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}

