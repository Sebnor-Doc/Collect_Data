#include "ui_TTS_GUI.h"
#include "TTS_GUI.h"
#include <QFileInfo>
#include <QSerialPortInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QUrl>
#include <QAudioEncoderSettings>
#include <QCloseEvent>
#include <QThread>
#include <QCameraInfo>
#include <QtXml>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
using boost::escaped_list_separator;
using boost::tokenizer;


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

    // Connect to Mojo
    QThread *magThread = new QThread(this);
    rs.moveToThread(magThread);

    connect(magThread, SIGNAL(started()), &rs, SLOT(process()));
    connect(this, SIGNAL(save(bool)), &rs, SLOT(saveMag(bool)));
    connect(this, SIGNAL(fileName(QString)), &rs, SLOT(setSubFilename(QString)));

    connect(this, SIGNAL(stopRecording()), &rs, SLOT(stop()));
    connect(&rs, SIGNAL(finished()), magThread, SLOT(quit()));
    connect(magThread, SIGNAL(finished()), magThread, SLOT(deleteLater()));

    magThread->start(QThread::HighestPriority);

    // Initialize Localization Thread
    QThread *locaThread = new QThread(this);
    loca.init(sensors, magnet);

    connect(locaThread, SIGNAL(started()), &loca, SLOT(start()));
    connect(&rs, SIGNAL(dataToLocalize(MagData)), &loca, SLOT(processMag(MagData)));
    connect(this, SIGNAL(fileName(QString)), &loca, SLOT(setFilename(QString)));

    locaThread->start();

    // Intialize video
    QThread *videoThread = new QThread(this);
    video.moveToThread(videoThread);

    connect(videoThread, SIGNAL(started()), &video, SLOT(process()));
    connect(this, SIGNAL(save(bool)), &video, SLOT(saveVideo(bool)));
    connect(ui->showVideoCheckBox, SIGNAL(clicked(bool)), &video, SLOT(displayVideo(bool)), Qt::DirectConnection);   
    connect(this, SIGNAL(fileName(QString)), &video, SLOT(setFilename(QString)));
    connect(&video, SIGNAL(processedImage(QPixmap)), ui->videoFeed, SLOT(setPixmap(QPixmap)));

    connect(this, SIGNAL(stopRecording()), &video, SLOT(stop()));
    connect(&video, SIGNAL(finished()), videoThread, SLOT(quit()));
    connect(videoThread, SIGNAL(finished()), videoThread, SLOT(deleteLater()));

    videoThread->start();
    ui->showVideoCheckBox->setChecked(true);
    emit ui->showVideoCheckBox->clicked(true);

    // Initialize Audio
    setAudio();
}


/* System and Session Configuration  */
void MainWindow::on_configButton_clicked()
{
    // Load calibration data and initialize sensors[] vector
    QString calibFilename = QString("TTS %1 - %2.xml").arg(ui->serialNumBox->currentText()).arg(ui->magnetTypeBox->currentText());
    loadCalibration(calibFilename);

    // Setup procedures
    setupExperiment();

    // Set state of buttons
    ui->measureEMFButton->setEnabled(true);
    ui->configButton->setEnabled(false);
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
    experiment_root = ui->subPathEdit->toPlainText() + "/Sub" + ui->subNbEdit->text();

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
    emit fileName(experiment_output_path);

    //Color the boxes
    QString formatUtterance = QString("<font size=\"34\" color=\"red\">%1</font>")
                              .arg(utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex()));
    ui->utteranceBrowser->setText(formatUtterance);

    emit save(true);

    audio1->stop();
    audio2->stop();
    audio1->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio1.wav"));
    audio2->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio2.wav"));
    audio1->record();
    audio2->record();
}

void MainWindow::stopTrial(){

    emit save(false);

    audio1->stop();
    audio2->stop();

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
        QString formatUtter = utter.at(index)->at(j) + "\t\t" + QString::number(j+1) + "/" + QString::number(utter.at(index)->size());
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

    experiment_utter = utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex());
    experiment_utter.truncate(40);
    experiment_utter = experiment_utter.trimmed();

    int trial = ui->trialBox->currentIndex() + 1;

    experiment_output_path = experiment_root + "/" + experiment_class + "/" + experiment_utter + "/" +
            experiment_utter + "_" + QString::number(trial) + "/" + experiment_utter + "_" +
            QString::number(trial);
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

/* Closing methods */
void MainWindow::closeEvent(QCloseEvent *event){

    emit stopRecording();

    // Terminate audio
    if (audio1) {
        audio1->stop();
    }

    if (audio2) {
       audio2->stop();
    }

    // Closing procedures
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
}

