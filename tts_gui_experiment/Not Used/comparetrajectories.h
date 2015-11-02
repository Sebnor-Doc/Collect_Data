#ifndef COMPARETRAJECTORIES_H
#define COMPARETRAJECTORIES_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include "spline.h"
#include <math.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "common.h"
#include <QLinearGradient>

using namespace std;

class CompareTrajectories: public QObject
{
    Q_OBJECT

public:
    CompareTrajectories();

public slots:
     void emitTrajectoryDifference();

signals:
    void emitPlotDetails(double rangeX, double rangeH, QLinearGradient *grad);
    void processedTrajectory(double time, double r, double expX, double expY, double expZ);

public:
    /* Instance Variables */
    std::vector<double> *time_ref, *x_ref, *y_ref, *z_ref;
    tk::spline s_ref_x, s_ref_y, s_ref_z;

    std::vector<double> *time_exp, *x_exp, *y_exp, *z_exp;
    tk::spline s_exp_x, s_exp_y, s_exp_z;

    std::vector<double> time_axis;
    std::vector<double> pr_traj;
    std::vector<double> x_est_traj;
    std::vector<double> y_est_traj;
    std::vector<double> z_est_traj;
    double max;
    QLinearGradient *grad;


    /* Methods */
    void loadReferenceFile(QString ref_file);
    void loadExperimentalFile(QString exp_file);
    std::vector<double> linspace(double a, double b, int n);
    void processTrajectory();

};

#endif // COMPARETRAJECTORIES_H
