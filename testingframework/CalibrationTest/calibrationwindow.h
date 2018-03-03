#ifndef CALIBRATIONWINDOW_H
#define CALIBRATIONWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimer>
#include <QFile>

#include "attentionspot.h"
#include "videocapture.h"


namespace Ui {
    class CalibrationWindow;
}

struct ListEntry
{
    int duration, snapshots;
    QPointF position;
};

struct CaptureFrame
{
    int frameNumber;
    QPointF position;
};

class CalibrationWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CalibrationWindow(QWidget *parent = 0);
    ~CalibrationWindow();

    void setTitle(QString title);
    void setVideoCapture(VideoCapture* cap);
    bool parsePointData(QString data);

public slots:
    void nextPoint();
    void centerPoint();
    void log(QPointF pos);

signals:
    void finished(QVector<CaptureFrame> result);

private:
    void setupDrawingarea();

    bool retry;


    QVector<ListEntry> entries;
    QVector<CaptureFrame> results;
    QGraphicsScene* scene;
    QGraphicsTextItem* windowTitle;
    Ui::CalibrationWindow *ui;

    AttentionSpot* spot;
    QTimer timer, updateTimer;
    VideoCapture* capture;
    int currentPoint;
};

#endif // CALIBRATIONWINDOW_H
