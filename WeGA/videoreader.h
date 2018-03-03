/*******************************************************************************

    videoreader.h - Last modified 12/30/2011
    Allows to read various movie-formats with the High-Level-API

    Part of the diploma-thesis "Head Position Invariant Gaze Tracking"
    Andreas Tarnowsky - andreas.tarnowsky@atsd.net/atarnows@welfenlab.de

    Part of the WeGA tracking library

*******************************************************************************/

#ifndef VIDEOREADER_H
#define VIDEOREADER_H

#include "gazeapi.h"

namespace WeGA {

class VideoReader : public FrameReader
{
public:
    VideoReader(const std::string& fileName);
    virtual ~VideoReader();

    virtual cv::Mat nextFrame();
    virtual bool hasNext();

private:
    cv::VideoCapture* cap;
};

}

#endif // VIDEOREADER_H
