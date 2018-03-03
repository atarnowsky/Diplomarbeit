#include "camerareader.h"

namespace WeGA {

CameraReader::CameraReader(unsigned int deviceID) :
    cap(new cv::VideoCapture(deviceID))
{
    /*
    // Future version: Allow modifying the image resolution
    cv::Size frameSize(1280,800);
    cap->set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
    cap->set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);
    */
}

CameraReader::~CameraReader()
{
    delete cap;
}

cv::Mat CameraReader::nextFrame()
{
    cv::Mat data;
    cap->read(data);
    cv::cvtColor(data, data, CV_BGR2GRAY);
    return data;
}

bool CameraReader::hasNext()
{
    return cap->isOpened();
}

}
