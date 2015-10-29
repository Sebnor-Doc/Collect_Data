#include "comparetrajectories.h"

CompareTrajectories::CompareTrajectories()
{
}

void CompareTrajectories::loadReferenceFile(QString ref_file)
{
    time_ref = new std::vector<double>();
    x_ref = new std::vector<double>();
    y_ref = new std::vector<double>();
    z_ref = new std::vector<double>();

    QFile ref(ref_file);
    QTextStream in(&ref);

    QStringList coords;
    bool firstLine = true;
    double time_start = 0;
    double time_end;

    if(ref.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        //Collect position, time data
        while(!in.atEnd())
        {
            coords = in.readLine().split(" ");
            if (firstLine)
            {
                time_start = coords[coords.length() - 1].toDouble();
                firstLine = false;
            }
            x_ref->push_back(coords[0].toDouble());
            y_ref->push_back(coords[1].toDouble());
            z_ref->push_back(coords[2].toDouble());
            time_ref->push_back(coords[coords.length() - 1].toDouble() - time_start);
        }

        //Normalize time axis so that speech signals can be compared
        time_end = time_ref->at(time_ref->size()-1);
        for (int i=0; i<time_ref->size(); i++)
        {
            time_ref->at(i) /= time_end;
        }

        s_ref_x.set_points(*time_ref,*x_ref);
        s_ref_y.set_points(*time_ref,*y_ref);
        s_ref_z.set_points(*time_ref,*z_ref);
    }
}

void CompareTrajectories::loadExperimentalFile(QString exp_file)
{
    time_exp = new std::vector<double>();
    x_exp = new std::vector<double>();
    y_exp = new std::vector<double>();
    z_exp = new std::vector<double>();

    QFile exp(exp_file);
    QTextStream in(&exp);

    QStringList coords;
    bool firstLine = true;
    double time_start = 0;
    double time_end;

    if(exp.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        while(!in.atEnd())
        {
            coords = in.readLine().split(" ");
            if (firstLine)
            {
                time_start = coords[coords.length() - 1].toDouble();
                firstLine = false;
            }
            x_exp->push_back(coords[0].toDouble());
            y_exp->push_back(coords[1].toDouble());
            z_exp->push_back(coords[2].toDouble());
            time_exp->push_back(coords[coords.length() - 1].toDouble() - time_start);
        }

        //Normalize time axis so that speech signals can be compared
        time_end = time_exp->at(time_exp->size()-1);
        for (int i=0; i<time_exp->size(); i++)
        {
            time_exp->at(i) /= time_end;
        }

        s_exp_x.set_points(*time_exp,*x_exp);
        s_exp_y.set_points(*time_exp,*y_exp);
        s_exp_z.set_points(*time_exp,*z_exp);
    }
}

void CompareTrajectories::emitTrajectoryDifference()
{
//    for (int i=0; i<time_axis.size(); i++)
//    {
//        emit processedTrajectory(time_axis.at(i), pr_traj.at(i),
//                                 x_est_traj.at(i),y_est_traj.at(i),z_est_traj.at(i));
//    }

    for (int i=25; i<time_exp->size()-25; i++)
    {
        emit processedTrajectory(time_exp->at(i), sqrt(pow(x_exp->at(i),2) + pow(y_exp->at(i),2) + pow(z_exp->at(i),2)),
                                 x_exp->at(i),y_exp->at(i),z_exp->at(i));
    }
}

void CompareTrajectories::processTrajectory()
{
    time_axis = linspace(time_ref->at(0),time_ref->at(time_ref->size()-1), NUM_PTS);
    pr_traj.resize(0);
    x_est_traj.resize(0);
    y_est_traj.resize(0);
    z_est_traj.resize(0);

    max = -1; // all r values are positive

    //Process Trajectory
    for (int i=0; i<time_axis.size(); i++)
    {
        //Store estimated trajectory in x,y,z
        x_est_traj.push_back(s_exp_x(time_axis.at(i)));
        y_est_traj.push_back(s_exp_y(time_axis.at(i)));
        z_est_traj.push_back(s_exp_z(time_axis.at(i)));

        //Measure difference between experimental trajectory and reference trajectory
        pr_traj.push_back((sqrt(pow(s_ref_x(time_axis.at(i))-s_exp_x(time_axis.at(i)),2.0) +
                                pow(s_ref_y(time_axis.at(i))-s_exp_y(time_axis.at(i)),2.0) +
                                pow(s_ref_z(time_axis.at(i))-s_exp_z(time_axis.at(i)),2.0))));
        if (pr_traj.at(i)>max)
            max = pr_traj.at(i);
    }

    //Process gradient colors
    double endTime = time_axis.at(time_axis.size()-1);
    grad = new QLinearGradient(0,0,endTime, 0);
    double color;

    for (int i=0; i<time_axis.size(); i++)
    {
       color = pr_traj.at(i)/max;
       int r = color*255;
       int g = (1-color)*255;

       r = (r <= 255) ? r : 255;
       r = (r >= 0) ? r : 0;

       g = (g <= 255) ? g : 255;
       g = (g >= 0) ? g : 0;

       grad->setColorAt(time_axis.at(i)/endTime, QColor(r,g,0));
    }

    //Setup plot
    emit emitPlotDetails(time_axis.at(time_axis.size()-1), max, grad);
}

vector<double> CompareTrajectories::linspace(double a, double b, int n)
{
    vector<double> array;
    double step = (b-a)/(n-1);

    while(a <= b) {
        array.push_back(a);
        a += step;           // could recode to better handle rounding errors
    }
    return array;
}
