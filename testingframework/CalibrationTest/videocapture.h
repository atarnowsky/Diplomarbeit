#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include <QString>
#include <QThread>
#include <QReadWriteLock>

class VideoCapture : public QThread
{
public:
    VideoCapture(int deviceNum, QString filename);
    ~VideoCapture();

    void run();
    void stop();

    int currentFrame();

private:
    void startLoop();

    QString _filename;
    int _device;
    int _frame;
    bool _quit;
    QReadWriteLock _lock, _stopLock;
};

#endif // VIDEOCAPTURE_H
