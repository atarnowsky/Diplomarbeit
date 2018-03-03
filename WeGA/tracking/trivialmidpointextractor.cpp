#include "trivialmidpointextractor.h"

namespace WeGA {

TrivialMidpointExtractor::TrivialMidpointExtractor()
{
}


TrivialMidpointExtractor::~TrivialMidpointExtractor()
{

}

cv::RotatedRect TrivialMidpointExtractor::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    return cv::RotatedRect(
                cv::Point(inputBinary.size().width / 2,
                          inputBinary.size().height / 2),
                cv::Size(inputBinary.size().width,
                         inputBinary.size().height),
                0
                );
}

}
