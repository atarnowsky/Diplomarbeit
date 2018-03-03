TARGET = mapping

INCLUDEPATH += ../../bin
LIBS += -L../../bin -lWeGA

#QMAKE_CFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
#QMAKE_CXXFLAGS_DEBUG+=-pg -g -DDEBUG -Wno-unused-parameter
#QMAKE_LFLAGS_DEBUG+=-pg -g

QMAKE_CFLAGS_DEBUG+=-O3 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_DEBUG+=-O3 -DNODEBUG -Wno-unused-parameter

QMAKE_CFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter
QMAKE_CXXFLAGS_RELEASE=-O3 -DNODEBUG -Wno-unused-parameter

SOURCES += main_invariant.cpp














