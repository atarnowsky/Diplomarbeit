#ifndef DEFAULTGLINTSELECTOR_H
#define DEFAULTGLINTSELECTOR_H

#include "interfaces.h"

namespace WeGA {

class DefaultGlintSelector : public RegionSelector
{
public:
    DefaultGlintSelector(const scalar& sizeFactor, vector2& pupilPosition);
    ~DefaultGlintSelector();

    virtual
    cv::Rect run(const cv::Mat& input, const cv::Mat& gray, cv::Mat& region);

private:
    cv::Point lastGlintPosition;
    const scalar& sizeFactor;
    vector2& pupil;
};

}

#endif // DEFAULTGLINTSELECTOR_H
