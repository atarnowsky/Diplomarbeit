#ifndef TRIVIALMIDPOINTEXTRACTOR_H
#define TRIVIALMIDPOINTEXTRACTOR_H

#include "interfaces.h"

namespace WeGA {

class TrivialMidpointExtractor : public MidpointExtractor
{
public:
    TrivialMidpointExtractor();
    ~TrivialMidpointExtractor();

    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray);
};

}

#endif // TRIVIALMIDPOINTEXTRACTOR_H
