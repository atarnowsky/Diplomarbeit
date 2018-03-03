#ifndef ENHANCEDLEASTSQUARES_H
#define ENHANCEDLEASTSQUARES_H

#include "interfaces.h"

namespace WeGA {

class BinaryLeastSquaresEllipseFitter : public MidpointExtractor
{
public:
    BinaryLeastSquaresEllipseFitter(vector2& glintPosition, const scalar& scaleFactor);

    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray);
protected:
    vector2& glint;
    const scalar& scaleFactor;
};

}

#endif // ENHANCEDLEASTSQUARES_H
