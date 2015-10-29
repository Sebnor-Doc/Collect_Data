#ifndef TTS_GUI_H
#define TTS_GUI_H

#include <QMainWindow>
#include <string>
#include <QCamera>

#define WIN32_LEAN_AND_MEAN // Necessary due to problems with Boost
#include "ReadSensors.h"
#include "Magnet.h"
#include "common.h"
//#include "videothread.h"

#include <QImage>

#include "CImg.h"
using namespace cimg_library;

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

/* **************************************************** *
 *              Variables                               *
 * **************************************************** */
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void on_configButton_clicked();
    void on_measureEMFButton_clicked();
    void on_showVideoCheckBox_clicked();

    void on_startStopTrialButton_toggled(bool checked);

    void on_classBox_currentIndexChanged(int index);

    void on_utteranceBox_currentIndexChanged(int index);

    void on_trialBox_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;
    QString calibration_xml;
    Magnet magnet;
    Sensor *sensors[NUM_OF_SENSORS];
    ReadSensors *rs;
    QCamera *camera;


    // String lists for Experimental Setup
    vector<QString> classUtter;
    vector<QStringList*> utter;
    vector<int> numTrials;
    int exp_count_p;

    // File locations
    QString experiment_root;
    QString experiment_class;
    QString experiment_utter;
    QString experiment_output_path;
    QString emfFile;


    //    QFile experiment_output_file;
    //    QTextStream *experiment_output_stream;


/* **************************************************** *
 *              Methods                                 *
 * **************************************************** */
private:
    void loadCalibration(Magnet& magnet, string pathCalibration);
    void setupExperiment();
    void loadExperimentFile(QString experimentFile);
    void setCamera();
    void beginTrial();
    void stopTrial();

    void setFilePath();
    CImg<double> loadVector(string myString);
    CImg<double> loadMatrix(string myString);


};



//#include <QtCore/QVariant>
//#include <QtWidgets/QAction>
//#include <QtWidgets/QApplication>
//#include <QtWidgets/QButtonGroup>
//#include <QtWidgets/QCheckBox>
//#include <QtWidgets/QGroupBox>
//#include <QtWidgets/QHeaderView>
//#include <QtWidgets/QLabel>
//#include <QtWidgets/QMainWindow>
//#include <QtWidgets/QMenu>
//#include <QtWidgets/QMenuBar>
//#include <QtWidgets/QStatusBar>
//#include <QtWidgets/QTabWidget>
//#include <QtWidgets/QToolBar>
//#include <QtWidgets/QVBoxLayout>
//#include <QtWidgets/QWidget>

//#include <boost/tokenizer.hpp>
//#include <boost/timer/timer.hpp>
//#include <boost/lexical_cast.hpp>

//#include "qcustomplot.h"
//#include "common.h"
//#include "roboticarmwrapper.h"
//#include <stdint.h>


//
//#include <QThread>

////Threads
//#include "VideoThread.h"
//#include "audiorecorder.h"

//#include "qaudiolevel.h"
//#include "LocalizationThread.h"

//#include "postprocessstreams.h"

//#include <QSound>

//// Session info Dialog
//#include "sessioninfodialog.h"


//// TOO MANY GLOBAL VARIABLES


//using namespace std;
//using namespace boost;
//using boost::escaped_list_separator;
//using boost::tokenizer;
//using boost::ref;

//namespace Ui {
//class MainWindow;
//}

//class MainWindow : public QMainWindow
//{
//    Q_OBJECT

///* **************************************************** *
// *              Variables                               *
// * **************************************************** */
//public:
//    VideoThread *video;
//    QTimer audioTimer;
//    AudioRecorder *audio1;
//    AudioRecorder *audio2;
//    QThread *audioThread1;
//    QThread *audioThread2;
//    LocalizationThread *lt;

//    //QTimer to collect information after saying something
//    double key_start, key_threshold, key_end;
//    double old_key;
//    double key;

//    //Post Processing
//    PostProcessStreams *ps;

//    QCustomPlot *sensorPlot;
//    typedef struct Plot {
//        QCPGraph *mainGraphX;
//        QCPGraph *mainGraphY;
//        QCPGraph *mainGraphZ;
//        QCPGraph *mainGraphXDot;
//        QCPGraph *mainGraphYDot;
//        QCPGraph *mainGraphZDot;
//        QCPAxisRect *wideAxisRect;
//        int check1, check2;
//        int working;
//        double key;
//        QCPCurve *parCurve;
//    } Plot;
//    Plot plots[NUM_OF_SENSORS];
//    QTimer sensorPlotTimer;
//    QTimer dataTimer_loc;
//    QTimer dataTimer_exp;
//    bool lead_dot, alias_fill;

//    //Feedback for slp - x vs y, x vs time, etc
//    QCustomPlot *posPlot;
//    Plot pPlots[6];
//    QTimer posPlotTimer;
//    QPen posPlotColors[6];

//    //States for tab index
//    enum {SENSOROUTPUTS, CALIBRATION, LOCALIZATION_ERROR, LOCALIZATION_EXPERIMENT};


//private:
//    Ui::MainWindow *ui;
//    int currentTab;
//    unsigned char buf[256];
//    QString input_calibration_file;
//    QString output_calibration_file;
//    QString input_trajectory_file;
//    QString output_trajectory_file;

//    //Experiment
//    SessionInfoDialog *infoDialog;
//    bool REFERENCE_AVAILABLE;
//    bool load_cal = true;
//    QString subNb;
//    QString experiment_output_path;
//    QString reference_input_path;

//    QString experiment_folder;
//    QString experiment_phrase;
//    QFile experiment_output_file;
//    QTextStream *experiment_output_stream;
//    int exp_count;
//    bool end_flag;

//    QStringList calibration;
//    QStringList words;


//    bool setupExperiment_log;
//    bool clearLog;

//    int calibration_count;
//    int words_count;
//    roboticArmWrapper *rW;

//    QLinearGradient *grad;

//    QString reference_location;

//

//    // Matlab VFB GUI file
//    string vfbMlFile;
//    QProcess visualFB;


///* **************************************************** *
// *              Methods                                 *
// * **************************************************** */
//public:
//    explicit MainWindow(QWidget *parent = 0);
//    ~MainWindow();
//    void setupPlots();
//    void setupPositionPlots();
//    Magnet localizer(Magnet &magnet);
//    Magnet localizer2(Magnet &magnet);
//    QString formatRefFilename(QString refPath);

//private:
//    void beginTrial();
//    void stopTrial();

//


//    void moveToNextTrial();

//    void updateMlGui(int selection);
//    void delay(int s);
//    void normalizeRawMag(QString rawMagFileLoc);

//public slots:
//    void junkData();
//    void graphSensorData();

//private slots:
//    void updatePlayerUI(QImage img);
//    void updateAudioLevels();
//    void updatePlotFeedback(double time, double r, double expX, double expY, double expZ);
//    void on_classBox_currentIndexChanged(int index);
//    void on_wordBox_currentIndexChanged(int index);
//    void on_playReferenceButton_clicked();
//    void on_checkBox_clicked();
//    void on_computeLocalization_clicked();
//    void on_expNoBox_currentIndexChanged(int index);
//    void on_tabWidget_tabBarClicked(int index);
//    void on_pushButton_clicked();
//    void on_startCalibrationButton_clicked();
//    void on_pushButton_3_clicked();
//    void on_pushButton_5_clicked();
//    void on_measureEMFButton_clicked();
//    void on_showVideoCheckBox_clicked();
//    void on_startStopTrialButton_toggled(bool checked);
//};

#endif // TTS_GUI_H
