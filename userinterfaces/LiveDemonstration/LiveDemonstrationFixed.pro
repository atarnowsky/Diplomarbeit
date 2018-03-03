#-------------------------------------------------
#
# Project created by QtCreator 2011-07-10T10:38:33
#
#-------------------------------------------------

QT       += core gui opengl

TARGET = LiveDemonstration
TEMPLATE = app

LIBS += -lWeGA
LIBS += -lopencv_core -lopencv_imgproc -lrt
LIBS += -lopencv_highgui -lopencv_ml -lopencv_video
LIBS += -lcvblobs

INCLUDEPATH += /usr/include/opencv2/

LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_contrib -lopencv_legacy -lopencv_flann

QMAKE_CFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter

SOURCES += main.cpp\
    calibrationwindow.cpp \
    attentionspot.cpp 

HEADERS  += calibrationwindow.h \
    attentionspot.h 

FORMS    += calibrationwindow.ui
