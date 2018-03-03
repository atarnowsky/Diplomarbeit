INCLUDEPATH += ../../bin
LIBS += -L../../bin -lWeGA

QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter -frounding-math
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter -frounding-math
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter -frounding-math
QMAKE_CXXFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter -frounding-math

SOURCES += main_fixed.cpp

visualTest {
    TARGET = algorithm_visual
    DEFINES += VISUAL_MODE
}

algorithmTest {
    TARGET = algorithm_list
    DEFINES += LIST_MODE
}

performanceTest {
    TARGET = algorithm_performance
    DEFINES += PERFORMANCE_MODE
}
