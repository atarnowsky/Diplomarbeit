#include "attentionspot.h"
#include <QPainter>
#include <QPointF>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

AttentionSpot::AttentionSpot(QObject *parent) :
    QGraphicsObject(0),
    movement(new QPropertyAnimation(this, "pos")),
    sizeani(new QPropertyAnimation(this, "size")),
    spotani(new QPropertyAnimation(this, "spot"))
{
    size = 50;
    spotintensity = 0;
    err = false;
    movement->setDuration(2000);
    movement->setEasingCurve(QEasingCurve::InOutSine);

    sizeani->setDuration(2000);
    sizeani->setEasingCurve(QEasingCurve::InOutCirc);

    spotani->setDuration(500);
    spotani->setEasingCurve(QEasingCurve::InOutSine);

    g = new QSequentialAnimationGroup();
    g->addAnimation(sizeani);
    g->addAnimation(spotani);

    connect(spotani, SIGNAL(currentLoopChanged(int)), this, SLOT(logSig()));
    //connect(spotani, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)), this, SLOT(logSig()));
}

QRectF AttentionSpot::boundingRect() const
{
    return QRectF(-100, -100, 100, 100);
}

void AttentionSpot::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem* option,
                          QWidget* widget)
{
    QRadialGradient g(QPointF(), size/2);
    g.setColorAt(0, QColor(255,err? 255 : 0,0,128 * size/250 + 64));
    g.setColorAt(1, Qt::transparent);
    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(QBrush(g));
    painter->drawEllipse(QPointF(), size/2, size/2);

    QRadialGradient g2(QPointF(), 5 + size /250);
    g2.setColorAt(1, QColor(255,err? 255 : 0,0));
    g2.setColorAt(0, Qt::transparent);
    painter->setBrush(QBrush(g2));
    painter->drawEllipse(QPointF(), 5 + size /25, 5 + size /25);

    QRadialGradient g3(QPointF(), 5);
    g3.setColorAt(0, err? Qt::red : Qt::white);
    g3.setColorAt(spotintensity, Qt::transparent);
    painter->setBrush(QBrush(g3));
    painter->drawEllipse(QPointF(), 5, 5);
}

void AttentionSpot::showSpot(QPointF pos, int duration, int snapshots)
{
    curPos = pos;
    movement->setKeyValueAt(0.0, this->pos());
    movement->setKeyValueAt(0.35, pos);
    movement->setKeyValueAt(1.0, pos);
    movement->start();

    sizeani->setKeyValueAt(0.0, 30);
    sizeani->setKeyValueAt(0.5, 30);
    sizeani->setKeyValueAt(0.7, 250);
    sizeani->setKeyValueAt(1.0, 30);

    spotani->setDuration(1000.0/(snapshots+1));
    spotani->setKeyValueAt(0.0, 1.0);
    spotani->setKeyValueAt(1.0, 0.0);
    spotani->setLoopCount(duration*snapshots);

    g->start();
}

void AttentionSpot::setSpot(QPointF pos)
{
    curPos = pos;
    this->setPos(pos);
    this->update();
}

void AttentionSpot::logSig()
{
    emit doLog(curPos);
}
