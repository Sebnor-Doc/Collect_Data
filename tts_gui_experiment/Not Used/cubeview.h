#ifndef CUBEVIEW_H
#define CUBEVIEW_H

#include "qglview.h"
#include "common.h"
#include "sensor.h"
#include "CImg.h"
#include "qglbuilder.h"
#include "qglcube.h"
#include "Magnet.h"
#include "qglcylinder.h"

QT_BEGIN_NAMESPACE
class QGLSceneNode;
QT_END_NAMESPACE

class CubeView : public QGLView
{
    Q_OBJECT
public:
    CubeView(QWindow *parent = 0, Sensor *sensors[NUM_OF_SENSORS] = 0);
    ~CubeView();

protected:
    void paintGL(QGLPainter *painter);
    void initializeGL(QGLPainter *painter);
    void setupSensors();

public:
    QGLSceneNode *scene;
    QGLSceneNode *cube;
    QGLSceneNode *lines[5];
    QGLSceneNode *sensorCubes[NUM_OF_SENSORS];
    QGLSceneNode *orientations[NUM_OF_SENSORS];
    Sensor *sensors[NUM_OF_SENSORS];
    QGLTexture2D *texture;
    QGLBuilder builder;
    Magnet nm;
};

#endif
