#include <QDebug>
#include "globalmouseapplication.h"
#include <X11/Xlib.h>

GlobalMouseApplication::GlobalMouseApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    timer.setInterval(0);
    connect(&timer, SIGNAL(timeout()), this, SLOT(checkForEvents()));

    timer.start();
}

GlobalMouseApplication::~GlobalMouseApplication()
{

}

void GlobalMouseApplication::checkForEvents()
{

}
