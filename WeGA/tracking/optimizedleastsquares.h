#ifndef OPTIMIZEDLEASTSQUARES_H
#define OPTIMIZEDLEASTSQUARES_H

#include "interfaces.h"

namespace WeGA {

class ComplexLeastSquaresEllipseFitter : public MidpointExtractor
{
public:
    ComplexLeastSquaresEllipseFitter(const scalar& scaleFactor,
                                     const scalar& usedThresh);

    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray);

protected:
    const scalar& scaleFactor;
    const scalar& usedThresh;
    cv::RotatedRect lastResult;
};
}
#endif // OPTIMIZEDLEASTSQUARES_H
