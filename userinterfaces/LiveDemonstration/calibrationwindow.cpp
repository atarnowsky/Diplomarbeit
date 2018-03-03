#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsTextItem>
#include <iostream>
#include <algorithm>
#include <QTime>
#include <QMessageBox>
#include <QBrush>
#include <cmath>

uint qHash(const QPointF& p)
{
    return (uint)qRound(p.x()*p.y()*10000);
}

CalibrationWindow::CalibrationWindow(QVector<ListEntry>& points,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CalibrationWindow)
{
    last = QPoint(-1,-1);
    entries = points;
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(qApp->desktop()->geometry());

    setupDrawingarea();

    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(nextPoint()));

    connect(&updateTimer, SIGNAL(timeout()), this->ui->view, SLOT(update()));
    connect(&cameraTimer, SIGNAL(timeout()), this, SLOT(nextCameraFrame()));
    cameraTimer.setInterval(5);
    updateTimer.setInterval(20);
    updateTimer.start();
    qApp->setOverrideCursor( QCursor( Qt::BlankCursor ) );

    cam = new WeGA::CameraReader(0);
    cam->nextFrame();

    WeGA::FixedHeadTracker::TrackingTunables tunables;
    tunables.pupilSize = 0.02;
    tunables.glintSize= -0.01;

    tracker = new WeGA::FixedHeadTracker(
                WeGA::FixedHeadTracker::ModifiedLeastSquares,
                 WeGA::FixedHeadTracker::Centroid, 0,
                "eye.png", tunables);

    mapper = new WeGA::FixedHeadMapper(WeGA::FixedHeadMapper::Linear);

    for(int n = 0; n < 20; n++)
    {
        cv::Mat img = cam->nextFrame();
        if(img.size().area() <= 0) return;
        WeGA::TrackingResult result = tracker->analyzeFrame(img);

        std::cout << result.toString() << std::endl;
    }
    std::cout << "---" << std::endl;
}

void CalibrationWindow::captureFrame(QPointF pos)
{
    cv::Mat img = cam->nextFrame();
    if(img.size().area() <= 0) return;
    WeGA::TrackingResult result = tracker->analyzeFrame(img);

    std::cout << result.toString() << std::endl;

    QPointF p(pos.x()/this->geometry().width(), pos.y()/this->geometry().height());

    training.insert(p, result);
}

void CalibrationWindow::updateInternal()
{
    spot->smoothMove(QCursor::pos());
    if(last != QPoint(-1,-1))
    {
        QPen p(QBrush(QColor(Qt::red)), 1.5);
        QGraphicsLineItem* i = scene->addLine(QLineF(QCursor::pos(), last), p);
        i->setZValue(0.25);
        lines.push_back(i);
        if(lines.count() > 120)
        {
            delete lines.front();
            lines.pop_front();
        }

        for(int n = 0; n < lines.size(); n++)
        {
            lines[n]->setOpacity(pow(1.0*n/lines.size(),1.0));
        }
    }

    last = QCursor::pos();
}

void CalibrationWindow::startCalibration()
{
    timer.start(1000);
}

void CalibrationWindow::startDemo()
{
    pix = new QGraphicsPixmapItem(0, scene);
    QPixmap p("wall.jpg");
    pix->setPixmap(p.scaledToWidth(this->geometry().width(), Qt::SmoothTransformation));
    pix->setPos(0, 0.5*(this->geometry().height() - pix->pixmap().height()));

    pix->setOpacity(0.0);
    pix->setZValue(-1);
    spot->setZValue(1);
    fadeTimer.setInterval(20);
    fadeTimer.start();
    connect(&fadeTimer, SIGNAL(timeout()), this, SLOT(fade()));
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateInternal()));

    std::vector<WeGA::FixedHeadMapper::TrainingPair> result;

    QPointF lastPoint(-999999,-999999);
    foreach(QPointF p, training.keys())
    {
        if(p == lastPoint) continue;
        QPointF gazePos(0,0);

        foreach(WeGA::TrackingResult result, training.values(p))
        {
            gazePos += QPointF(result.pupilGlintVector().x,
                               result.pupilGlintVector().y);
        }
        gazePos /= training.count(p);

        QPointF stdDev(0.0, 0.0);
        foreach(WeGA::TrackingResult result, training.values(p))
        {
            QPointF valDiff(QPointF(result.pupilGlintVector().x,
                                    result.pupilGlintVector().y) - gazePos);
            stdDev.setX(stdDev.x() + valDiff.x()*valDiff.x());
            stdDev.setY(stdDev.y() + valDiff.y()*valDiff.y());
        }
        stdDev.setX(sqrt(stdDev.x()/(training.count(p)-1)));
        stdDev.setY(sqrt(stdDev.y()/(training.count(p)-1)));

        QPointF gazePosOld = gazePos;
        gazePos = QPointF(0,0);
        int counter = 0;
        foreach(WeGA::TrackingResult result, training.values(p))
        {
            if((fabs(result.pupilGlintVector().x - gazePosOld.x()) <= stdDev.x())
                    && (fabs(result.pupilGlintVector().y - gazePosOld.y()) <= stdDev.y()))
            {
                gazePos += QPointF(result.pupilGlintVector().x,
                                   result.pupilGlintVector().y);
                counter++;
            }
        }
        gazePos /= counter;
        std::cout << "# Removed " << training.count(p) - counter << " calibration points." << std::endl;

        WeGA::vector2 corners[4] = {WeGA::vector2(0,0),
                                    WeGA::vector2(0,0),
                                    WeGA::vector2(0,0),
                                    WeGA::vector2(0,0)};
        WeGA::TrackingResult::CornerStatus status[4] = {
            WeGA::TrackingResult::Missing, WeGA::TrackingResult::Missing,
            WeGA::TrackingResult::Missing, WeGA::TrackingResult::Missing
        };

        WeGA::FixedHeadMapper::TrainingPair e;
        e.first = WeGA::TrackingResult(WeGA::vector2(gazePos.x(), gazePos.y()), WeGA::vector2(0,0),
                                 corners, status, 1.0);
        e.second = WeGA::MappingResult(WeGA::vector2(p.x(), p.y()), false);
        result.push_back(e);

        lastPoint = p;
    }

    mapper->addTrainingData(result);
    //mapper->setKernelWidth(0.65* 1.0/mapper->trainingDataDiameter());
    mapper->sphericOptimization();

    cameraTimer.start();
}

void CalibrationWindow::nextCameraFrame()
{
    cv::Mat img = cam->nextFrame();
    if(img.size().area() <= 0) return;
    WeGA::TrackingResult result = tracker->analyzeFrame(img);
    WeGA::MappingResult mappingRes = mapper->mapToScreen(result);

    QPointF point(mappingRes.position().x * this->geometry().width(),
                   mappingRes.position().y * this->geometry().height());

    lastPoints.push_back(point);
    if(lastPoints.count() > 10)
        lastPoints.pop_front();

    QPointF avg(0,0);
    foreach (QPointF p, lastPoints)
    {
        avg += p;
    }
    avg /= lastPoints.count();

    QPointF stdDev(0,0);
    foreach (QPointF p, lastPoints)
    {
        QPointF diff = avg - p;
        stdDev += QPointF(diff.x()*diff.x(),diff.y()*diff.y());
    }
    stdDev /= lastPoints.count() - 1;
    stdDev.setX(sqrt(stdDev.x()));
    stdDev.setY(sqrt(stdDev.y()));

    QPointF realPoint(0,0);
    int count = 0;
    foreach (QPointF p, lastPoints)
    {
        if((fabs(p.x() - avg.x()) < stdDev.x())
                && (fabs(p.y() - avg.y()) < stdDev.y()))
        {
            realPoint += p;
            count++;
        }
    }
    realPoint /= count;



    QCursor::setPos(realPoint.x(), realPoint.y());
}

void CalibrationWindow::fade()
{
    pix->setOpacity(pix->opacity() + 0.025);
    if(pix->opacity() >= 1.0)
        fadeTimer.stop();
}


CalibrationWindow::~CalibrationWindow()
{
    scene->deleteLater();
    delete ui;
}

void CalibrationWindow::setupDrawingarea()
{
    scene = new QGraphicsScene(qApp->desktop()->geometry());
    ui->view->setScene(scene);
    ui->view->setSceneRect(qApp->desktop()->geometry());
    ui->view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);

    spot = new AttentionSpot();
    scene->addItem(spot);
    spot->setPos(this->geometry().center());

    connect(spot, SIGNAL(doLog(QPointF)), this, SLOT(captureFrame(QPointF)));
}

void CalibrationWindow::nextPoint()
{
    if(entries.empty())
    {
        startDemo();
        return;
    }

    ListEntry nextOne = entries.front();
    entries.pop_front();

    QPointF p(nextOne.position.x() * this->geometry().width(),
              nextOne.position.y() * this->geometry().height());

    spot->showSpot(p, nextOne.duration, nextOne.snapshots);
    timer.start(1000*(nextOne.duration + spot->duration()));
}

