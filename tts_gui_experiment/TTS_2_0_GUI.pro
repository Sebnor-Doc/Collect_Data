#-------------------------------------------------
#
# Project created by Nordine Sebkhi
#
#-------------------------------------------------

QT      += core gui printsupport widgets serialport multimedia multimediawidgets
CONFIG  += console

TARGET = TTS_NEU
TEMPLATE = app


SOURCES += main.cpp\
        TTS_GUI.cpp \
    qcustomplot.cpp \
    pugixml.cpp \
    mojoserialport.cpp \
    Magnet.cpp \
    Sensor.cpp \
    ReadSensors.cpp \
    sensordisplay.cpp

HEADERS  += TTS_GUI.h \
    qcustomplot.h \
    common.h \
    pugixml.hpp \
    pugiconfig.hpp \
    CImg.h \
    mojoserialport.h \
    Magnet.h \
    Sensor.h \
    ReadSensors.h \
    sensordisplay.h

FORMS    += \
    TTS_GUI.ui \
    sensordisplay.ui

LIBS += -lAdvapi32 -lgdi32 -luser32 -lshell32

INCLUDEPATH += "C:/dev/boost_1_57_0-0"
INCLUDEPATH += "C:/dev/opencv/build/include"

INCLUDEPATH += $$PWD/../../../../../../dev/boost_1_57_0-0/lib64-msvc-12.0
DEPENDPATH += $$PWD/../../../../../../dev/boost_1_57_0-0/lib64-msvc-12.0

unix|win32: LIBS += -L$$PWD/../../../../../../dev/boost_1_57_0-0/lib64-msvc-12.0/ -lboost_system-vc120-mt-gd-1_57
unix|win32: LIBS += -L$$PWD/../../../../../../dev/boost_1_57_0-0/lib64-msvc-12.0/ -lboost_regex-vc120-mt-gd-1_57

