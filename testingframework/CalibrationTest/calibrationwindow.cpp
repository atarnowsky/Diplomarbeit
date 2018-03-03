#include "calibrationwindow.h"
#include "ui_calibrationwindow.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsTextItem>
#include <iostream>
#include <algorithm>
#include <QTime>

CalibrationWindow::CalibrationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CalibrationWindow),
    capture(0)
{
    srand(QTime::currentTime().msec() + QTime::currentTime().second() * 1000);
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(qApp->desktop()->geometry());

    setupDrawingarea();

    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(nextPoint()));

    connect(&updateTimer, SIGNAL(timeout()), this->ui->view, SLOT(update()));
    updateTimer.setInterval(10);
    updateTimer.start();

    retry = false;
}

CalibrationWindow::~CalibrationWindow()
{
    scene->deleteLater();
    delete ui;
}

void CalibrationWindow::setVideoCapture(VideoCapture* cap)
{
    capture = cap;
}

void CalibrationWindow::setupDrawingarea()
{
    scene = new QGraphicsScene(qApp->desktop()->geometry());
    ui->view->setScene(scene);
    ui->view->setSceneRect(qApp->desktop()->geometry());
    ui->view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);


    windowTitle = scene->addText("");
    windowTitle->setDefaultTextColor(Qt::white);
    windowTitle->setTextWidth(this->geometry().width());

    spot = new AttentionSpot();
    scene->addItem(spot);
    spot->setPos(this->geometry().center());

    connect(spot, SIGNAL(doLog(QPointF)), this, SLOT(log(QPointF)));
}

void CalibrationWindow::setTitle(QString title)
{
    windowTitle->setHtml(title);
}

bool CalibrationWindow::parsePointData(QString data)
{
    entries.clear();
    results.clear();
    currentPoint = 0;

    QStringList lines(data.split("\n", QString::SkipEmptyParts));

    foreach(QString line, lines)
    {
        // Skip comments
        if(line.trimmed().at(0) == QChar('#'))
            continue;

        QStringList parts(line.split(";"));

        if(parts.count() != 4)
            return false;

        ListEntry e;
        bool ok = true;
        e.snapshots = parts.at(3).toInt(&ok);
        if(!ok)
            return false;

        e.duration = parts.at(2).toInt(&ok);
        if(!ok)
            return false;

        e.position.setX(parts.at(0).toDouble(&ok));
        if(!ok)
            return false;

        e.position.setY(parts.at(1).toDouble(&ok));
        if(!ok)
            return false;

        entries.push_back(e);
    }

    std::random_shuffle(entries.begin(), entries.end());

    return entries.count() > 0;
}

void CalibrationWindow::nextPoint()
{
    if(retry)
    {
        timer.start(1000*(entries[currentPoint].duration + spot->duration()));
        retry = false;
        return;
    }

    if(currentPoint >= entries.count())
    {
        currentPoint = 0;
        emit finished(results);
        return;
    }

    QPointF p(entries[currentPoint].position.x() * this->geometry().width(),
              entries[currentPoint].position.y() * this->geometry().height());

    spot->showSpot(p, entries[currentPoint].duration, entries[currentPoint].snapshots);
    timer.start(1000*(entries[currentPoint].duration + spot->duration()));
    currentPoint++;
}

void CalibrationWindow::centerPoint()
{
    QPointF p(0.5 * this->geometry().width(),
              0.5 * this->geometry().height());

    spot->setSpot(p);
}

void CalibrationWindow::log(QPointF pos)
{    
    if(!capture) return;
    CaptureFrame frame;
    frame.frameNumber = capture->currentFrame();
    frame.position = entries[currentPoint-1].position;
    std::cout << frame.frameNumber << ";"
              << frame.position.x() << ";" << frame.position.y() << ";"
              << std::endl;
    results.push_back(frame);
}
