INCLUDEPATH += ../../bin
LIBS += -L../../bin -lWeGA

QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter

SOURCES += main_invariant.cpp

visualTestInvariant {
    TARGET = algorithm_visual_invariant
    DEFINES += VISUAL_MODE
}

algorithmTestInvariant {
    TARGET = algorithm_list_invariant
    DEFINES += LIST_MODE
}

performanceTestInvariant {
    TARGET = algorithm_performance_invariant
    DEFINES += PERFORMANCE_MODE
}
