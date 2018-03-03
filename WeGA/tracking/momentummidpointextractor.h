#ifndef MOMENTUMMIDPOINTEXTRACTOR_H
#define MOMENTUMMIDPOINTEXTRACTOR_H

#include "interfaces.h"

namespace WeGA {

class MomentumMidpointExtractor : public MidpointExtractor
{
public:
    MomentumMidpointExtractor(bool invert = false);
    ~MomentumMidpointExtractor();

    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray);

private:
    bool invert;
};

}

#endif // MOMENTUMMIDPOINTEXTRACTOR_H
