#-------------------------------------------------
#
# Project created by Nordine Sebkhi
#
#-------------------------------------------------

TARGET      = MSCS
VERSION     = 2.5
TEMPLATE    = app


QT      += core gui printsupport widgets serialport multimedia multimediawidgets xml
CONFIG  += console static


SOURCES += main.cpp\
        TTS_GUI.cpp \
    qcustomplot.cpp \
    Magnet.cpp \
    Sensor.cpp \
    ReadSensors.cpp \
    sensordisplay.cpp \
    videothread.cpp \
    localization.cpp \
    voicemanager.cpp \
    vfbmanager.cpp

HEADERS  += TTS_GUI.h \
    qcustomplot.h \
    common.h \
    Magnet.h \
    Sensor.h \
    ReadSensors.h \
    sensordisplay.h \
    videothread.h \
    localization.h \
    voicemanager.h \
    vfbmanager.h \
    typedef.h

FORMS    += \
    TTS_GUI.ui \
    sensordisplay.ui

LIBS += -lAdvapi32 -lgdi32 -luser32 -lshell32

RESOURCES += \
    icons.qrc

# Boost Libraries
INCLUDEPATH += "C:/dev/Boost/boost_1_59_0"  # Needed for the boost/... includes
DEPENDPATH  += "C:/dev/Boost/boost_1_59_0/stage/lib"
LIBS += -LC:/dev/Boost/boost_1_59_0/stage/lib

# OPEN CV
INCLUDEPATH += "C:/dev/OpenCV/OpenCV_3.0_Installer/opencv/build/include"

LIBS += -L"C:/dev/OpenCV/OpenCV_3.0_Installer/Build_Shared_32_bit/lib/Release" \
            -lopencv_flann300       \
            -lopencv_hal300         \
            -lopencv_highgui300     \
            -lopencv_imgcodecs300   \
            -lopencv_imgproc300     \
            -lopencv_ml300          \
            -lopencv_objdetect300   \
            -lopencv_photo300       \
            -lopencv_shape300       \
            -lopencv_stitching300   \
            -lopencv_superres300    \
            -lopencv_ts300          \
            -lopencv_video300       \
            -lopencv_videoio300     \
            -lopencv_videostab300   \
            -lopencv_calib3d300     \
            -lopencv_core300        \
            -lopencv_features2d300


