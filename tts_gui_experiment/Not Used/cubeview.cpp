#include "cubeview.h"
#include <QtCore/qurl.h>
#include <QFile>

CubeView::CubeView(QWindow *parent,  Sensor *sensors[NUM_OF_SENSORS])
    : QGLView(parent)
    , cube(0)
    , texture(0)
{


    //Setup an origin
    builder.newSection(QGL::Faceted);
    QGLCylinder mag =  QGLCylinder(.5,.5,.25,32,100);
    mag.setBaseEnabled(false);
    mag.setTopEnabled(false);
    builder << mag;
    cube = builder.currentNode();


    // Draw a fake trajectory between 5 points

    QGLCylinder l[5];
    for (int i=0; i<5; i++)
    {
        l[i] = QGLCylinder(.1,.1,.25,32,100);
        l[i].setBaseEnabled(false);
        l[i].setTopEnabled(false);
        builder.newSection(QGL::Faceted);
        builder << l[i];
        lines[i] = builder.currentNode();
    }


//    cube->setEffect(QGL::LitDecalTexture2D);
//    QGLMaterial *mat = new QGLMaterial;
//    mat->setColor(QColor(170, 202, 0));
//    cube->setMaterial(mat);
    setupSensors();
    for (int i=0; i<NUM_OF_SENSORS; i++)
    {
        builder.newSection(QGL::Faceted);
        builder << QGLCube(.5f);
        sensorCubes[i] = builder.currentNode();

        builder.newSection(QGL::Faceted);
        builder << QGLCube(.5f/4);
        orientations[i] = builder.currentNode();
    }

    scene = builder.finalizedSceneNode();
    scene->setParent(this);
}

void CubeView::setupSensors()
{
    QFile input("C:/Users/Shurjo/Documents/Research2014/TTS_2_0_GUI/SensorInput/Sensors24_Positions_Angles.txt");
    QTextStream in(&input);
    QStringList s_pos;

    if (input.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        for (int i=0; i<NUM_OF_SENSORS;i++)
        {
            sensors[i] = new Sensor();
            s_pos = in.readLine().split(" ");
            // Write sensor positions in metres
            sensors[i]->m_position.fill(s_pos[0].toDouble(),s_pos[1].toDouble(),s_pos[2].toDouble());
            // Write in sensor orientation
            sensors[i]->m_angles.fill(s_pos[3].toDouble(),s_pos[4].toDouble(),s_pos[5].toDouble());
            sensors[i]->id = i;
            //set gain
            sensors[i]->m_gain(0,0) = .32e-3;
            sensors[i]->m_gain(1,1) = .32e-3;
            sensors[i]->m_gain(2,2) = .32e-3;
        }
    }
}



CubeView::~CubeView()
{
    texture->cleanupResources();
    delete cube;
}

void CubeView::paintGL(QGLPainter *painter)
{
    painter->setStandardEffect(QGL::LitMaterial);
    painter->setFaceColor(QGL::AllFaces, QColor(170, 202, 0));
    painter->modelViewMatrix().translate(nm.m_position(0),nm.m_position(1),nm.m_position(2));
    cube->draw(painter);

    painter->modelViewMatrix().translate(-nm.m_position(0),-nm.m_position(1),-nm.m_position(2));
    double scale = 100;

    //Draw a fake trajectory

    painter->setStandardEffect(QGL::LitMaterial);
    painter->setFaceColor(QGL::AllFaces, QColor(0, 0, 255));

    painter->modelViewMatrix().translate(0,0,-.25);
    lines[0]->draw(painter);
    painter->modelViewMatrix().translate(0,0,.25);

    painter->modelViewMatrix().translate(0,0,-.50);
    lines[0]->draw(painter);
    painter->modelViewMatrix().translate(0,0,.50);

    painter->modelViewMatrix().translate(0,0,-.75);
    lines[0]->draw(painter);
    painter->modelViewMatrix().translate(0,0,.75);

    for (int i=0; i<5; i++)
    {
        painter->modelViewMatrix().translate(0,0,-.25*i);
        lines[0]->draw(painter);
        painter->modelViewMatrix().translate(0,0,.25*i);
    }


    for (int i=0; i<5; i++)
    {
        painter->modelViewMatrix().rotate(90,0,1,0);
        painter->modelViewMatrix().translate(1.,0,-.25*i-.1);
        lines[0]->draw(painter);
        painter->modelViewMatrix().translate(-1.,0,.25*i+.1);
        painter->modelViewMatrix().rotate(-90,0,1,0);
    }



    for (int i=0; i<NUM_OF_SENSORS; i++)
    {
        painter->setStandardEffect(QGL::LitMaterial);
        painter->setFaceColor(QGL::AllFaces, Qt::black);
        painter->modelViewMatrix().translate(sensors[i]->m_position(0)*scale, sensors[i]->m_position(1)*scale, sensors[i]->m_position(2)*scale);
        painter->modelViewMatrix().rotate(sensors[i]->m_angles(0),0,0,1);
        painter->modelViewMatrix().rotate(sensors[i]->m_angles(1),1,0,0);
        painter->modelViewMatrix().rotate(sensors[i]->m_angles(2),0,0,1);
        sensorCubes[i]->draw(painter);
        painter->setStandardEffect(QGL::LitMaterial);
        painter->setFaceColor(QGL::AllFaces, Qt::white);
        painter->modelViewMatrix().translate(0.5f/2,0.5f/2,0.5f/2);
        orientations[i]->draw(painter);
        painter->modelViewMatrix().translate(-0.5f/2,-0.5f/2,-0.5f/2);
        painter->modelViewMatrix().rotate(-sensors[i]->m_angles(2),0,0,1);
        painter->modelViewMatrix().rotate(-sensors[i]->m_angles(1),1,0,0);
        painter->modelViewMatrix().rotate(-sensors[i]->m_angles(0),0,0,1);
        painter->modelViewMatrix().translate(-sensors[i]->m_position(0)*scale, -sensors[i]->m_position(1)*scale, -sensors[i]->m_position(2)*scale);
    }

}

void CubeView::initializeGL(QGLPainter *painter)
{
    painter->setClearColor(Qt::gray);
}
