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

    // Disable buttons to enforce proper config sequence
    ui->startStopTrialButton->setEnabled(false);
    ui->measureEMFButton->setEnabled(false);

    // Connect to Mojo
    rs = new ReadSensors();

    QSerialPortInfo mojoComPort;
    QList<QSerialPortInfo> listOfPorts = QSerialPortInfo::availablePorts();

    foreach(QSerialPortInfo port, listOfPorts) {

        if (port.productIdentifier() == 32769) {
            mojoComPort = port;
            break;
        }
    }

    rs->setCOMPort(mojoComPort.portName());
    rs->Play();

    // TO BE REPLACED BY EXTERNAL XML FILE
    ui->expFileEdit->setText("C:/Users/nsebkhi3/GitHub/GTBionics - TTS/Data_Collection_NEU/Experiments/NEU_Experiments.txt");
    ui->ttsSystEdit->setText("C:/Users/nsebkhi3/GitHub/GTBionics - TTS/Data_Collection_NEU/Calibration/TTS 002-BT - Large Magnet.xml");
    ui->subPathEdit->setText("C:/TTS_Data/Test");
    ui->subNbEdit->setText("99");


    // Initialize Media
    setCamera();
    setAudio();

}

// Setup the experiment
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

            for (int k = 0; k < numTrials.at(i); k++)
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
        ui->classBox->addItem(classUtter.at(i) + "\t\t" + QString::number(i+1) + "/" + QString::number(classUtter.size()));
    }

    // Update utterance display
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(ui->utteranceBox->currentIndex())
                                  + QString("</font>"));

    // Set instance variables for folder/file paths
    setFilePath();
}


void MainWindow::loadCalibration(Magnet& magnet, string pathCalibration) {

    // Create the root of the tree
    pugi::xml_document doc;
    doc.load_file(pathCalibration.c_str());

    // Create a child node from "calibration"
    pugi::xml_node nodeCalibration = doc.child("calibration");

    // Set Magnet's instance variables to data from calibration file
    pugi::xml_node nodeMagnet = nodeCalibration.child("magnet");
    magnet.diameter(boost::lexical_cast<double>(nodeMagnet.child("diameter").child_value()));
    magnet.length(boost::lexical_cast<double>(nodeMagnet.child("length").child_value()));
    magnet.Br(boost::lexical_cast<double>(nodeMagnet.child("Br").child_value()));
    magnet.Bt(boost::lexical_cast<double>(nodeMagnet.child("Bt").child_value()));

    // Display Magnet attributes (diameter, length, Br, Bt, theta, phi, moment, position)
//    magnet.print();

    // Instantiate Sensor objects and set their instance variables (id, position , gain, ...)
    int i = 0;
    for (pugi::xml_node nodeSensor = nodeCalibration.child("sensor"); nodeSensor; nodeSensor = nodeSensor.next_sibling("sensor"))
    {
        sensors[i] = new Sensor();
        sensors[i]->id = nodeSensor.attribute("id").as_uint();
        sensors[i]->position(loadVector(nodeSensor.child("position").child_value()));
        sensors[i]->EMF(loadVector(nodeSensor.child("EMF").child_value()));
        sensors[i]->offset(loadVector(nodeSensor.child("offset").child_value()));
        sensors[i]->gain(loadMatrix(nodeSensor.child("gain").child_value()));
        sensors[i]->angles(loadVector(nodeSensor.child("angles").child_value()));
        //sensors[i]->print();  // Print sensor parameters
        i++;
    }

}// end loadCalibration method

/**
 * Set utter, classUtter and numTrials vectors from a list of utterances in the experiment file
 * @param experimentFile File path of list of utterances
 */
void MainWindow::loadExperimentFile(QString experimentFile)
{
    QFile input(experimentFile);
    QTextStream in(&input);
    in.setCodec("UTF-8");

    QString line;
    exp_count_p = -1;

    if (input.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while(!in.atEnd())
        {
            line = in.readLine();

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
                exp_count_p++;
            }

            else if (line.contains("Iter", Qt::CaseInsensitive))
            {
                int ind = line.indexOf(':');
                numTrials.push_back(((line.mid(ind+1)).trimmed()).toInt());
            }

            else
            {
                utter.at(exp_count_p)->append(line.trimmed());
            }
        }
    }
    else {
        qDebug() << "Cannot open experiment file located at:\n" << experimentFile << endl;
    }
}



void MainWindow::on_configButton_clicked()
{
    // Load calibration data and initialize sensors[] vector
    calibration_xml = ui->ttsSystEdit->text();
    loadCalibration(magnet, calibration_xml.toStdString());

    // Setup procedures
    setupExperiment();

    // Set state of buttons
    ui->measureEMFButton->setEnabled(true);
    ui->configButton->setEnabled(false);
}

void MainWindow::on_measureEMFButton_clicked()
{
    // Set EMF file path
    QString currentDateTime = QDateTime::currentDateTime().toString("yy-MM-dd_hh-mm");
    emfFile = experiment_root + "/EMF_" + currentDateTime + ".txt";

    QFile outputEMF(emfFile);
    QTextStream output_stream(&outputEMF);

    // Read magnetic values
    int num_iterations = 100;

    if (outputEMF.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        for (int iter = 0; iter < num_iterations; iter++)
        {

            for (int i = 0; i < NUM_OF_SENSORS; i++)
            {
                int x = rs->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 0);
                int y = rs->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 1);
                int z = rs->getSensorData(i/NUM_OF_SENSORS_PER_BOARD, i%NUM_OF_SENSORS_PER_BOARD, 2);

                sensors[i]->m_EMF += cimg_library::CImg<double>().assign(1,3).fill(x,y,z);
            }
        }

        // Take the average
        for (int i = 0; i < NUM_OF_SENSORS; i++)
        {
            sensors[i]->m_EMF = sensors[i]->m_EMF / num_iterations;
            output_stream << sensors[i]->m_EMF(0) << " " << sensors[i]->m_EMF(1) << " " << sensors[i]->m_EMF(2) << endl;
        }
        outputEMF.close();
    }

    // Update buttons status
    ui->measureEMFButton->setText("EMF Measured!");
    ui->measureEMFButton->setEnabled(false);
    ui->startStopTrialButton->setEnabled(true);
}

/* Graph Sensor data */
void MainWindow::on_showMagButton_toggled(bool checked)
{
    if (checked) {
        ui->showMagButton->setText("Hide Sensors");
        sensorUi = new SensorDisplay(this, rs);
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
void MainWindow::setCamera(){
    // Find LifeCam webcam and assign it to "camera" variable
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    foreach (const QCameraInfo &cameraInfo, cameras) {

        if (cameraInfo.description().contains("LifeCam")) {
            camera = new QCamera(cameraInfo);
            qDebug() << "LifeCam connected!";
            break;
        }
    }

    // Show debug message if camera cannot be found
    if (!camera) {
        QString errorMsg = "LifeCam camera cannot be found";
        qDebug() << errorMsg << endl;
    }

    camera->setCaptureMode(QCamera::CaptureVideo);
}

void MainWindow::on_showVideoCheckBox_clicked()
{
    if (ui->showVideoCheckBox->isChecked())
    {
        camera->start();
    }
    else
    {
        camera->stop();
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

    if (sender() == audioProbe1) {
        ui->leftVolumeBar->setValue(0);
    } else {
        ui->rightVolumeBar->setValue(0);
    }


}

/* Data collection */
void MainWindow::on_startStopTrialButton_toggled(bool checked)
{
    if (checked) {
        ui->startStopTrialButton->setText("Stop");       
        beginTrial();

    } else {       
        stopTrial();
        ui->startStopTrialButton->setText("Start");
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
    rs->setFileLocationAndName(experiment_output_path + "_raw_sensor.txt"); // Magnetic stream
    rs->saveToFile();

    audio1->stop();
    audio2->stop();
    audio1->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio1.wav"));
    audio2->setOutputLocation(QUrl::fromLocalFile(experiment_output_path + "_audio2.wav"));
    audio1->record();
    audio2->record();

    //    video->setVideoName(experiment_output_path + "_video.avi");             // Video stream

//    lt->setFileLocationAndName(experiment_output_path + "_localization.txt");  // Tongue Trajectory
//    lt->setSensorFile(experiment_output_path + "_raw_sensor.txt");

    // Post processing files
//    ps->setReferenceFile(reference_input_path + "_localization.txt");
//    ps->setExperimentalFile(experiment_output_path + "_localization.txt");


    // Begin Recording data
//    key_start = QDateTime::currentDateTime().toMSecsSinceEpoch();
//    audio1->beginSavingAudio();
//    audio2->beginSavingAudio();
//    video->startVideo();


//    lt->saveToFile();

}

void MainWindow::stopTrial(){

    rs->stopSavingToFile();

    audio1->stop();
    audio2->stop();

}

/* Manage Drop-down lists */
void MainWindow::on_classBox_currentIndexChanged(int index)
{
    disconnect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));

    // Clear utterance and trial drop-down list
    ui->utteranceBox->clear();
    ui->trialBox->clear();

    // Populate list of utterances for this class
    for(int j = 0; j < utter.at(index)->size(); j++)
    {
        QString formatUtter = utter.at(index)->at(j) + "\t\t" + QString::number(j+1) + "/" + QString::number(utter.at(index)->size());
        ui->utteranceBox->addItem(formatUtter);
    }

    // Populate list of trial numbers for this class
    for (int i = 0; i < numTrials.at(index); i++)
    {
        ui->trialBox->addItem(QString::number(i+1) + " / " + QString::number(numTrials.at(index)));
    }

    // Update utterrance display
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(0) + QString("</font>"));

    // Reset drop-down list to first item
    ui->utteranceBox->setCurrentIndex(0);
    ui->trialBox->setCurrentIndex(0);

    connect(ui->utteranceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(on_utteranceBox_currentIndexChanged(int)));
}

void MainWindow::on_utteranceBox_currentIndexChanged(int index)
{
    ui->trialBox->setCurrentIndex(0);
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(index) + QString("</font>"));

//    // Update saving paths
//    setFilePath();
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

    exp_count_p = ui->trialBox->currentIndex() + 1;

    experiment_output_path = experiment_root + "/" + experiment_class + "/" + experiment_utter + "/" +
            experiment_utter + "_" + QString::number(exp_count_p) + "/" + experiment_utter + "_" +
            QString::number(exp_count_p);
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
    audio1->stop();
    audio2->stop();
    event->accept();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete camera;
}






