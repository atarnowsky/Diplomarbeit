#include <QtGui/QApplication>
#include "calibrationwindow.h"
#include <iostream>
#include <QFile>
#include <QMessageBox>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QVector<ListEntry> spots;

    ListEntry e;
    e.duration = 2;
    e.snapshots = 8;
    e.position = QPointF(0.5,0.5);
    spots.append(e);
    e.position = QPointF(0.05,0.05);
    spots.append(e);
    e.position = QPointF(0.5,0.05);
    spots.append(e);
    e.position = QPointF(0.95,0.05);
    spots.append(e);
    e.position = QPointF(0.95,0.5);
    spots.append(e);
    e.position = QPointF(0.95,0.95);
    spots.append(e);
    e.position = QPointF(0.5,0.95);
    spots.append(e);
    e.position = QPointF(0.05,0.95);
    spots.append(e);
    e.position = QPointF(0.05,0.5);
    spots.append(e);

    CalibrationWindow w(spots);
    w.show();

    QMessageBox::information(&w, "Waiting for user", "Press enter to start calibration");

    w.startCalibration();

    return a.exec();
}
