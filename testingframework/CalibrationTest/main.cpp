#include <QtGui/QApplication>
#include "calibrationwindow.h"
#include <iostream>
#include <QFile>
#include "videocapture.h"
#include "profilemaskwindow.h"

#define ERR_INVALIDARG -1
#define ERR_FILEREAD   -2
#define ERR_FILEPARSE  -3
#define ERR_DEVICEREAD -4
#define ERR_DEVICEMAP  -5



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("CalibrationTestSuite");
    a.setOrganizationName("Welfenlab_Thesis_atarnows");

    ProfilemaskWindow m;
    m.show();

    return a.exec();
}
