#-------------------------------------------------
#
# Project created by Nordine Sebkhi
#
#-------------------------------------------------

TARGET      = MSCS
VERSION     = 3.0
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
    vfbmanager.cpp \
    patientdialog.cpp

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
    typedef.h \
    patientdialog.h

FORMS    += \
    TTS_GUI.ui \
    sensordisplay.ui \
    patientdialog.ui

LIBS += -lAdvapi32 -lgdi32 -luser32 -lshell32

RESOURCES += \
    icons.qrc


# Matlab Score generation libraries
INCLUDEPATH += "C:/Program Files/MATLAB/MATLAB Runtime/v85/extern/include"
LIBS        += "C:/Program Files/MATLAB/MATLAB Runtime/v85/extern/lib/win64/microsoft/mclmcrrt.lib"

INCLUDEPATH += "C:/dev/MSCS_Score/Localization"
LIBS        += "C:/dev/MSCS_Score/Localization/LocalizationScore.lib"

INCLUDEPATH += "C:/dev/MSCS_Score/Audio"
LIBS        += "C:/dev/MSCS_Score/Audio/AudioScore.lib"

INCLUDEPATH += "C:/dev/MSCS_Score/Magnetic"
LIBS        += "C:/dev/MSCS_Score/Magnetic/MagneticScore.lib"

INCLUDEPATH += "C:/dev/MSCS_Score/Video"
LIBS        += "C:/dev/MSCS_Score/Video/VideoScore.lib"


# Boost Libraries
INCLUDEPATH += "C:/dev/Boost/boost_1_60_0"
DEPENDPATH  += "C:/dev/Boost/boost_1_60_0/stage/Lib_static_msvc12_64bit"
LIBS        += -LC:/dev/Boost/boost_1_60_0/stage/Lib_static_msvc12_64bit


# OPEN CV
INCLUDEPATH += "C:/dev/OpenCV/OpenCV_3_0/opencv/build/include"
LIBS        += -L"C:/dev/OpenCV/OpenCV_3_0/Build_Shared_64bit/lib/Release" \
                -lopencv_calib3d300     \
                -lopencv_core300        \
                -lopencv_features2d300  \
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
                -lopencv_video300       \
                -lopencv_videoio300     \
                -lopencv_videostab300




