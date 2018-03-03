#include "videoreader.h"

namespace WeGA {

VideoReader::VideoReader(const std::string& fileName) :
    cap(new cv::VideoCapture(fileName))
{
}

VideoReader::~VideoReader()
{
    delete cap;
}

cv::Mat VideoReader::nextFrame()
{
    cv::Mat data;
    cap->read(data);
    if(data.size().area() > 0)
    {
        cv::cvtColor(data, data, CV_BGR2GRAY);
    }
    return data;
}

bool VideoReader::hasNext()
{
    return cap->isOpened();
}

}
