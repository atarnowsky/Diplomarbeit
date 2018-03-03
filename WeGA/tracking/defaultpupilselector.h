#ifndef DEFAULTPUPILSELECTOR_H
#define DEFAULTPUPILSELECTOR_H

#include "interfaces.h"

namespace WeGA {

class DefaultPupilSelector : public RegionSelector
{
public:
    DefaultPupilSelector(const scalar& sizeFactor);
    ~DefaultPupilSelector();

    virtual
    cv::Rect run(const cv::Mat& input, const cv::Mat& gray, cv::Mat& region);

private:
    cv::Point lastPupilPosition;
    const scalar& sizeFactor;
    scalar avgDarkness;
    scalar avgFill;
    scalar avgArea;
};

}

#endif // DEFAULTPUPILSELECTOR_H
