#include <QDebug>
#include "x11mousewidget.h"
#include <X11/Xlib.h>

X11MouseWidget::X11MouseWidget(QWidget *parent) :
    QWidget(parent)
{
}

bool X11MouseWidget::x11EventFilter(XEvent* event)
{
    qDebug() << event->type;
    return false;
}
