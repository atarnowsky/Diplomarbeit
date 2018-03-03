#include "momentummidpointextractor.h"
#include <cmath>

namespace WeGA {

MomentumMidpointExtractor::MomentumMidpointExtractor(
    bool invert) :
    invert(invert)
{

}

MomentumMidpointExtractor::~MomentumMidpointExtractor()
{

}

cv::RotatedRect MomentumMidpointExtractor::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    if(inputBinary.size().area() == 0)
        return cv::RotatedRect();

    cv::Mat invGray = inputGray.clone();

    if(invert)
    {
        cv::Mat matMax(invGray.size(), invGray.type(), CV_RGB(255, 255, 255));
        invGray = matMax - invGray;
    }

    cv::Moments m = cv::moments(invGray);

    cv::RotatedRect result;

    result.center.x = m.m10/m.m00;
    result.center.y = m.m01/m.m00;

    return result;

}

}
