/*******************************************************************************

    camerareader.h - Last modified 12/30/2011
    Allows to use a (USB-)camera device with the High-Level-API

    Part of the diploma-thesis "Head Position Invariant Gaze Tracking"
    Andreas Tarnowsky - andreas.tarnowsky@atsd.net/atarnows@welfenlab.de

    Part of the WeGA tracking library

*******************************************************************************/

#ifndef CAMERAREADER_H
#define CAMERAREADER_H

#include "gazeapi.h"

namespace WeGA {

class CameraReader : public FrameReader
{
public:
    CameraReader(unsigned int deviceID);
    virtual ~CameraReader();

    virtual cv::Mat nextFrame();
    virtual bool hasNext();

private:
    cv::VideoCapture* cap;
};

}

#endif // CAMERAREADER_H
