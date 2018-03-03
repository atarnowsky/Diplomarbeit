#ifndef X11MOUSEWIDGET_H
#define X11MOUSEWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class X11MouseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit X11MouseWidget(QWidget *parent = 0);
    bool x11EventFilter(XEvent* event);

signals:
    void mouseClicked(QMouseEvent translatedEvent);

};

#endif // X11MOUSEWIDGET_H
