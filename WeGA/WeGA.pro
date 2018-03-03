#-------------------------------------------------
#
# Project created by QtCreator 2011-09-18T16:59:25
#
#-------------------------------------------------

QT       -= core gui

TARGET = WeGA
TEMPLATE = lib
CONFIG += lib

INCLUDEPATH += rbfnet
LIBS += -lopencv_core -lopencv_imgproc -lrt
LIBS += -lopencv_highgui -lopencv_ml -lopencv_video -lCGAL -lgmp
LIBS += -lcvblobs

QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter -frounding-math
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter -frounding-math
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter -frounding-math -march=native
QMAKE_CXXFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter -frounding-math -march=native

SOURCES += wega.cpp \
    gazeapi.cpp \
    tracking/trivialmidpointextractor.cpp \
    tracking/momentummidpointextractor.cpp \    
    tracking/leastsquaresellipsefitter.cpp \
    tracking/histthreshold.cpp \
    tracking/fulltemplatelocator.cpp \
    tracking/enhancedleastsquares.cpp \
    tracking/defaultpupilselector.cpp \
    tracking/defaultglintselector.cpp \
    camerareader.cpp \
    videoreader.cpp \
    tracking/optimizedleastsquares.cpp \
    invariantTracking/defaultcornerextractor.cpp \
    invariantTracking/cornersorter.cpp \
    delaunay/linearinterpolator.cpp

HEADERS += wega.h \
    gazeapi.h \
    tracking/trivialmidpointextractor.h \
    tracking/momentummidpointextractor.h \    
    tracking/leastsquaresellipsefitter.h \
    tracking/histthreshold.h \
    tracking/fulltemplatelocator.h \
    tracking/enhancedleastsquares.h \
    tracking/defaultpupilselector.h \
    tracking/defaultglintselector.h \
    interfaces.h \
    camerareader.h \
    videoreader.h \
    rbfnet/rbfapi.h \
    rbfnet/interpolationtrainer.h \
    rbfnet/defaultmetrics.h \
    rbfnet/defaultbasisfunctions.h \
    tracking/optimizedleastsquares.h \
    invariantTracking/defaultcornerextractor.h \
    invariantTracking/cornersorter.h \
    rbfnet/approximationtrainer.h \
    rbfnet/randomdomainplacer.h \
    rbfnet/kmeansplacer.h \
    rbfnet/rolfplacer.h \
    delaunay/linearinterpolator.h
    

header_files.files = wega.h \
    gazeapi.h \
    interfaces.h \
    camerareader.h \
    videoreader.h \
    rbfnet/rbfapi.h \
    rbfnet/interpolationtrainer.h \
    rbfnet/defaultmetrics.h \
    rbfnet/defaultbasisfunctions.h \
    rbfnet/approximationtrainer.h \
    rbfnet/randomdomainplacer.h

header_files.path = ../../bin/WeGA
target.path = ../../bin

INSTALLS += header_files
INSTALLS += target







































