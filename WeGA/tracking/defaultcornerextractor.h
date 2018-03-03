#ifndef DEFAULTCORNEREXTRACTOR_H
#define DEFAULTCORNEREXTRACTOR_H

#include "gazeapi.h"

class DefaultCornerExtractor
{
public:
    DefaultCornerExtractor(float scaleFactor);
    CornerSet operator() (const cv::Mat& inputBinary,
                          const cv::Mat& inputGray,
                          const cv::RotatedRect& ellipse,
                          float& confidence);

private:
    float scaleFactor;
};

#endif // DEFAULTCORNEREXTRACTOR_H
