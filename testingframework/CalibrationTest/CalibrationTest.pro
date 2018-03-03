#-------------------------------------------------
#
# Project created by QtCreator 2011-07-10T10:38:33
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = CalibrationTest
TEMPLATE = app

INCLUDEPATH += /usr/include/opencv2/
LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann


QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O2 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O2 -DNODEBUG -Wno-unused-parameter

SOURCES += main.cpp\
        calibrationwindow.cpp \
    attentionspot.cpp \
    videocapture.cpp \
    profilemaskwindow.cpp

HEADERS  += calibrationwindow.h \
    attentionspot.h \
    videocapture.h \
    profilemaskwindow.h

FORMS    += calibrationwindow.ui \
    profilemaskwindow.ui
