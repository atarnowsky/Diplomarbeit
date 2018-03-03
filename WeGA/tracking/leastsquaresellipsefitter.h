#ifndef LEASTSQUARESELLIPSEFITTER_H
#define LEASTSQUARESELLIPSEFITTER_H

#include "interfaces.h"

namespace WeGA {

class LeastSquaresEllipseFitter : public MidpointExtractor
{
public:
    LeastSquaresEllipseFitter();
    ~LeastSquaresEllipseFitter();

    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray);
};

}

#endif // LEASTSQUARESELLIPSEFITTER_H
