#ifndef GLOBALMOUSEAPPLICATION_H
#define GLOBALMOUSEAPPLICATION_H

#include <QApplication>
#include <QMouseEvent>
#include <QWidget>
#include <QTimer>

class GlobalMouseApplication : public QApplication
{
    Q_OBJECT
public:
    GlobalMouseApplication(int &argc, char **argv);
    virtual ~GlobalMouseApplication();

signals:
    void mouseClicked(QMouseEvent translatedEvent);

protected slots:
    void checkForEvents();

private:
    QTimer timer;
};

#endif // GLOBALMOUSEAPPLICATION_H
