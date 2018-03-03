#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QFile>
#include <QMultiHash>

#include "attentionspot.h"

#include "WeGA/wega.h"
#include "WeGA/camerareader.h"

namespace Ui {
    class CalibrationWindow;
}

struct ListEntry
{
    int duration, snapshots;
    QPointF position;
};

class CalibrationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CalibrationWindow(QVector<ListEntry>& points, QWidget *parent = 0);
    ~CalibrationWindow();

    void startCalibration();
    void startDemo();

public slots:
    void nextPoint();
    void fade();
    void nextCameraFrame();
    void updateInternal();

    void captureFrame(QPointF);

private:
    void setupDrawingarea();
    QVector<ListEntry> entries;
    QGraphicsScene* scene;
    Ui::CalibrationWindow *ui;

    AttentionSpot* spot;
    QGraphicsPixmapItem* pix;
    QTimer timer, updateTimer, cameraTimer, fadeTimer;
    QPoint last;
    QVector<QGraphicsLineItem*> lines;
    int currentPoint;

    WeGA::CameraReader* cam;
    WeGA::FixedHeadTracker* tracker;
    WeGA::FixedHeadMapper* mapper;


    QVector<QPointF> lastPoints;
    QMultiHash<QPointF, WeGA::TrackingResult> training;
};

#endif // CALIBRATIONWINDOW_H
