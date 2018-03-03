#ifndef ATTENTIONSPOT_H
#define ATTENTIONSPOT_H

#include <QGraphicsObject>
#include <QPropertyAnimation>
#include <QGraphicsBlurEffect>

class AttentionSpot : public QGraphicsObject
{
    Q_OBJECT
    Q_PROPERTY(double size READ circSize WRITE setCircSize)
    Q_PROPERTY(double spot READ spot WRITE setSpot)
public:
    explicit AttentionSpot(QObject *parent = 0);

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = 0);

    int duration() { return 2; }

    double circSize() { return size; }
    void setCircSize(double v) { size = v; }

    double spot() { return spotintensity; }
    void setSpot(double v) { spotintensity = v; }

    void setErr(bool value) { err = value; }


    void smoothMove(QPointF p);

signals:
    void doLog(QPointF pos);

public slots:
    void showSpot(QPointF pos, int duration, int snapshots);
    void setSpot(QPointF pos);
    void logSig();

private:
    float size, spotintensity;
    QPointF curPos;
    bool err;

    QPropertyAnimation *movement, *sizeani, *spotani;
    QSequentialAnimationGroup* g;

};

#endif // ATTENTIONSPOT_H
