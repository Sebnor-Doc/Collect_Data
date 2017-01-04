#include "localization.h"

#include <QDebug>
#include <QVector3D>
#include <QVector2D>
#include <qmath.h>

/************
 * Localization
 * ********** */

Localization::Localization(QObject *parent) : QObject(parent) {

}

void Localization::init(QVector<Sensor*> sensors, Magnet &magnet)
{
    locaWorker.init(sensors, magnet);
}

void Localization::start(){

    QThread *locaWorkerThread = new QThread(this);

    locaWorker.moveToThread(locaWorkerThread);

    connect(locaWorkerThread, SIGNAL(started()), &locaWorker, SLOT(start()));
    connect(this, SIGNAL(packetToLoca(MagData, QString)), &locaWorker, SLOT(localize(MagData, QString)), Qt::QueuedConnection);
    connect(&locaWorker, SIGNAL(dataLocalized(LocaData)), this, SLOT(processLoca(LocaData)));

    locaWorkerThread->start();
}

void Localization::processMag(MagData magData){

    emit packetToLoca(magData, filename);
}

void Localization::processLoca(LocaData locaData){

    if (QString::compare(outputFile.fileName(), locaData.filename) != 0) {
        outputFile.close();
        outputFile.setFileName(locaData.filename);
        outputStream.setDevice(&outputFile);
        outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
    }

    outputStream << locaData.x << " "
                 << locaData.y << " "
                 << locaData.z << " "
                 << locaData.theta << " "
                 << locaData.phi << " "
                 << locaData.time << " "
                 << locaData.id << endl;

    emit packetLocalized(locaData);
}

void Localization::setFilename(QString fileRoot){

    // Reset Text Stream object
    outputFile.close();
    filename = fileRoot + "_loca.txt";
    outputFile.setFileName(filename);
    outputStream.setDevice(&outputFile);
    outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
}


/************
 * LocaWorker
 * ********** */
LocaWorker::LocaWorker(QObject *parent) : QObject(parent) {
}

void LocaWorker::init(QVector<Sensor*> sensors, Magnet &magnet)
{
    this->magnet = magnet;

    localizer = new Localizer(this);
    localizer->init(sensors, &magnet);

    Matx<double, 1, 5> initStep(0.0001, 0.0001, 0.0001, CV_PI/720.0, CV_PI/720.0);
    TermCriteria criter(TermCriteria::COUNT + TermCriteria::EPS, 1000000, 0.0000000001);

    nealderMead = DownhillSolver::create(localizer, initStep, criter);
}

void LocaWorker::start()
{
}

void LocaWorker::localize(MagData origData, QString filename) {

    localizer->setMagData(origData);

    QVector3D lastMagnetPos = magnet.getPosition();
    QVector2D lastMagnetAngle = magnet.getAngles();

    Matx<double_t, 1, 5> initPoint(lastMagnetPos.x(), lastMagnetPos.y(), lastMagnetPos.z(), lastMagnetAngle.x(), lastMagnetAngle.y());

    double res = nealderMead->minimize(initPoint);

    LocaData locaPoint;
    locaPoint.x = initPoint(0);
    locaPoint.y = initPoint(1);
    locaPoint.z = initPoint(2);
    locaPoint.theta = initPoint(3);
    locaPoint.phi = initPoint(4);
    locaPoint.id = origData.id;
    locaPoint.time = origData.time;
    locaPoint.filename = filename;

    magnet.setPosition(initPoint(0), initPoint(1), initPoint(2));
    magnet.setMoment(initPoint(3), initPoint(4));

    emit dataLocalized(locaPoint);
}


/************
 * Localizer
 * ********** */

Localizer::Localizer(QObject *parent) : QObject(parent) {

}

void Localizer::init(QVector<Sensor*> sensors, Magnet *magnetPtr) {
    this->sensors = sensors;
    magnet = magnetPtr;
}

void Localizer::setMagData(MagData &magData){
    this->magData = magData;
}

double Localizer::calc(const double *x) const {

    double error = 0.0;

    magnet->setPosition(x[0], x[1], x[2]);
    magnet->setMoment(x[3], x[4]);

    for (int i = 0; i < NUM_OF_SENSORS; i++) {

        int Bx = magData.packet.at(3*i);
        int By = magData.packet.at(3*i + 1);
        int Bz = magData.packet.at(3*i + 2);

        sensors[i]->updateMagField(Bx, By, Bz);

        error += ( magnet->getTheoField(sensors[i]->getPosition()) - sensors[i]->getMagField() ).lengthSquared();
    }

    return error;
}

int Localizer::getDims() const {
    return 5;
}























//Localization::Localization(ReadSensors *rs, QVector<Sensor*> &sensors, Magnet &magnet, QObject *parent) : QObject(parent)
//{
//    stop = false;
//    save = false;
//    this->rs = rs;
//    this->sensors = sensors;
//    this->magnet = magnet;
//}




////Magnet magnet;
////QVector<Sensor*> sensors;

////static double localizationErrorFunction(double x[5]) {

////    Magnet testMagnet(magnet);
////    testMagnet.position(CImg<double>().assign(1,3).fill(x[0],x[1],x[2]));
////    testMagnet.moment(x[3],x[4]);

////    double error = 0;
////    for(int i=0; i<NUM_OF_SENSORS; i++)
////    {
////        error += ( testMagnet.fields(sensors[i]->position()) - sensors[i]->m_extrapField ).magnitude();
////    }

////    return error;
////}



//void Localization::computeLocalization(MagData magData)
//{

//    mutex.lock();

//    // Update sensor fields
//    for (int i = 0; i < NUM_OF_SENSORS; i++)
//    {
//        int Bx = magData.at(i*3);
//        int By = magData.at(i*3 + 1);
//        int Bz = magData.at(i*3 + 2);

//        sensors[i]->updateCurrentField(Bx, By, Bz);
//        QVector<int> magField = sensors[i]->getCurrentField();

//        (*outputFile_stream) << magField.at(0) << " "
//                             << magField.at(1) << " "
//                             << magField.at(2) << " ";

//    }
//    (*outputFile_stream) << endl;


//    mutex.unlock();



//    //

//    //





//    //    // Localize
//    //    magnet = localizer(magnet);

//    //    // Save localized magnet data
//    //    (*outputFile_stream) << magnet.m_position(0) << " "
//    //                         << magnet.m_position(1) << " "
//    //                         << magnet.m_position(2) << " "
//    //                         << magnet.m_theta       << " "
//    //                         << magnet.m_phi         << " "
//    //                         << QDateTime::currentDateTime().toMSecsSinceEpoch() << endl;



//}


//Magnet Localization::localizer(Magnet &magnet)
//{
//    const int n = 5;
//    double start[n] = {magnet.position()(0),magnet.position()(1),magnet.position()(2),magnet.theta(),magnet.phi()};
//    double step[n] = {0.0001,0.0001,0.0001,PI/720.,PI/720.};
//    double reqmin = 1.0E-10;
//    double xmin[n];
//    int konvge = 250;
//    int kcount = 500;
//    int icount;
//    int ifault;
//    int numres;
////    double ynewlo = localizationErrorFunction(start);

////    double (Localization::*errorFunc_ptr)(double[5]);
////    errorFunc_ptr = &Localization::localizationErrorFunction;

////    nelmin ( localizationErrorFunction, n, start, xmin, &ynewlo, reqmin, step, konvge, kcount, &icount, &numres, &ifault );
////    Magnet newMagnet(magnet);
////    newMagnet.position(CImg<double>().assign(1,3).fill(xmin[0],xmin[1],xmin[2])).moment(xmin[3],xmin[4]);
////    return newMagnet;
//    Magnet newMagnet(magnet);
//    return newMagnet;
//}


///* Manage Saving localization data  */
//void Localization::setFileLocation(QString filename)
//{
//    this->filename = filename;
//}

//void Localization::startSaving()
//{
//    mutex.lock();
//    save = true;
//    outputFile.setFileName(filename);
//    outputFile_stream = new QTextStream(&outputFile);
//    outputFile.open(QIODevice::WriteOnly | QIODevice::Text);
//    mutex.unlock();

//    connect(rs, SIGNAL(packetSaved(MagData)), this, SLOT(computeLocalization(MagData)));

//}

//void Localization::stopSaving()
//{
//    disconnect(rs, SIGNAL(packetSaved(MagData)), this, SLOT(computeLocalization(MagData)));
//    mutex.lock();
//    save = false;

//    if(outputFile.isOpen()){
//        outputFile.close();
//    }

//    mutex.unlock();
//}


///* Stop Execution */
//void Localization::Stop()
//{
//    stopSaving();
//    stop = true;
//}

//Localization::~Localization()
//{
//    stop = true;
//}






