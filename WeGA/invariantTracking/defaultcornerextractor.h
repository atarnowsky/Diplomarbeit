#ifndef DEFAULTCORNEREXTRACTOR_H
#define DEFAULTCORNEREXTRACTOR_H

#include "interfaces.h"

namespace WeGA {

class DefaultCornerExtractor : CornerExtractor
{
public:
    DefaultCornerExtractor(float scaleFactor);
    std::vector<vector2> run(const cv::Mat& inputBinary,
                             const cv::Mat& inputGray);

private:
    float scaleFactor;
};

}

#endif // DEFAULTCORNEREXTRACTOR_H
