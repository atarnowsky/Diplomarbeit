#ifndef FULLTEMPLATELOCATOR_H
#define FULLTEMPLATELOCATOR_H

#include <opencv2/opencv.hpp>
#include "interfaces.h"

namespace WeGA {

class FullTemplateLocator : public EyeLocator
{
public:
    FullTemplateLocator(cv::Mat& templ);
    ~FullTemplateLocator();

    virtual
    cv::Rect run(const cv::Mat& input);

private:
    cv::Mat templateImage;
    cv::Size frameSize;
    cv::Rect lastValid;
};

}

#endif // FULLTEMPLATELOCATOR_H
