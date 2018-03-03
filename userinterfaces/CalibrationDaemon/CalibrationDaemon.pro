#-------------------------------------------------
#
# Project created by QtCreator 2011-09-08T10:53:06
#
#-------------------------------------------------

TARGET = CalibrationDaemon
TEMPLATE = app

SOURCES += main.cpp

INCLUDEPATH += /usr/include/opencv2/
LIBS += -L/home/atarnows/localLib/opencv2/lib/ -lopencv_core -lopencv_imgproc -lrt
LIBS += -lopencv_highgui -lopencv_ml -lopencv_video
LIBS += -lX11 -lXtst

QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O3 -ffast-math -march=native -msse3 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O3 -ffast-math -march=native -msse3 -DNODEBUG -Wno-unused-parameter
