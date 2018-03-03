TARGET = CameraCalibration

INCLUDEPATH += /usr/include/opencv2/
LIBS += -lopencv_core -lopencv_imgproc -lrt
LIBS += -lopencv_highgui -lopencv_ml -lopencv_video

QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O2 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O2 -DNODEBUG -Wno-unused-parameter

SOURCES += \
    main.cpp
