#include "videocapture.h"
#include <opencv.hpp>
#include <opencv2/legacy/compat.hpp>
#include <highgui/highgui.hpp>

#include <iostream>

VideoCapture::VideoCapture(int deviceNum, QString filename) :
    _filename(filename), _device(deviceNum),
    _frame(0), _quit(false)
{

}

VideoCapture::~VideoCapture()
{
    stop();
    this->terminate();
}

void VideoCapture::run()
{
    startLoop();
}

void VideoCapture::stop()
{
    _stopLock.lockForWrite();
    _quit = true;
    _stopLock.unlock();
}

int VideoCapture::currentFrame()
{
    int val;
    _lock.lockForRead();
    val = _frame;
    _lock.unlock();
    return val;
}

void VideoCapture::startLoop()
{
    cv::Size frameSize(1280, 800);
    cv::VideoCapture device(_device);

    device.set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
    device.set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);
    //device.set(CV_CAP_PROP_SHARPNESS, 0);

    cv::VideoWriter writer(QString(_filename + ".avi").toStdString(), CV_FOURCC('H','F','Y','U'),
                           1, frameSize, true);
    if(!device.isOpened() || !writer.isOpened())
        return;

    std::cout << "Successfully initialized camera reader and HuffYUV writer"
              << std::endl;

    bool quitNow = false;
    while(!quitNow && device.isOpened() && writer.isOpened())
    {
        cv::Mat image, gray;
        device >> image;        
        cv::cvtColor(image, gray, CV_BGR2GRAY);
        cv::cvtColor(gray, image, CV_GRAY2RGB);
        writer << image;
        _lock.lockForWrite();
        _frame++;
        _lock.unlock();
        _stopLock.lockForRead();
        quitNow = _quit;
        _stopLock.unlock();
    }
}
