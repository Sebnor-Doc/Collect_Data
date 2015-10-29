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
#include <QCameraInfo>
#include <QCameraViewfinder>


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
    ui->expFileEdit->setText("C:/Users/nsebkhi3/GitHub/GTBionics - TTS/Data_Collection_NEU/NEU_Experiments.txt");
    ui->ttsSystEdit->setText("C:/Users/nsebkhi3/GitHub/GTBionics - TTS/Data_Collection_NEU/tts_gui_experiment/temp/calibration - TTS001BT - largemagnet.xml");
    ui->subPathEdit->setText("C:/TTS_Data/Test");
    ui->subNbEdit->setText("99");


    //Initialize Video
    setCamera();


//    video = new VideoThread();
//    video->Play();

//    ps = new PostProcessStreams();
//    ps->numTrials = expRepeat.at(0);

//    //Initialize Audio Threads (currently not on a thread..)
//    audio1 = new AudioRecorder(0,0);
//    audio2 = new AudioRecorder(0,1);
//    audioThread1 = new QThread;
//    audioThread2 = new QThread;
//    audio1->moveToThread(audioThread1);
//    audio2->moveToThread(audioThread2);
//    audioThread1->start();
//    audioThread2->start();

//    //Localization Thread
//    lt = new LocalizationThread(0, rs);


    // Connect Signals to Slots
//    connect(&audioTimer, SIGNAL(timeout()), this, SLOT(updateAudioLevels()));
//    connect(video, SIGNAL(processedImage(QImage)), this, SLOT(updatePlayerUI(QImage)));
//    connect(ps->ct, SIGNAL(processedTrajectory(double, double, double, double, double)), this, SLOT(updatePlotFeedback(double, double, double, double, double)));
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
    for (int j = 0; j < utter.at(0)->size(); j++)
    {
        ui->utteranceBox->addItem((utter.at(0)->at(j)+ "\t\t" + QString::number(j+1) + "/" + QString::number(utter.at(0)->size())));
    }
    for (int k = 0; k < numTrials.at(0); k++)
    {
        ui->trialBox->addItem(QString::number(k+1) + " / " + QString::number(numTrials.at(0)));
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
    camera->setViewfinder(ui->cameraPlayer);

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


void MainWindow::on_startStopTrialButton_toggled(bool checked)
{
    if (checked) {
        ui->startStopTrialButton->setText("Stop");
        beginTrial();

    } else {
        ui->startStopTrialButton->setText("Start");
        stopTrial();
    }
}


void MainWindow::on_classBox_currentIndexChanged(int index)
{
    // Clear utterance and trial drop-down list
    ui->utteranceBox->clear();
    ui->trialBox->clear();

    // Populate list of utterances for this class
    for(int j = 0; j < utter.at(index)->size(); j++)
    {
        ui->utteranceBox->addItem(utter.at(index)->at(j));
    }

    // Populate list of trial numbers for this class
    for (int i = 0; i < numTrials.at(index); i++)
    {
        ui->trialBox->addItem(QString::number(i+1));
    }

    // Update utterrance display
    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(0) + QString("</font>"));

    // Update saving paths
    setFilePath();
}

void MainWindow::on_utteranceBox_currentIndexChanged(int index)
{
    ui->trialBox->setCurrentIndex(0);

    ui->utteranceBrowser->setText(QString("<font size=\"40\">") + utter.at(ui->classBox->currentIndex())->at(index) + QString("</font>"));

    // Update saving paths
    setFilePath();
}

void MainWindow::on_trialBox_currentIndexChanged(int index)
{

}


void MainWindow::beginTrial(){

}

void MainWindow::stopTrial(){

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

/* Destructor */
MainWindow::~MainWindow()
{
    delete ui;
    delete camera;
}










