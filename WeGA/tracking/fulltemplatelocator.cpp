#include "fulltemplatelocator.h"

namespace WeGA {

const int scaleDown = 8;

FullTemplateLocator::FullTemplateLocator(
        cv::Mat& templ) :
    templateImage(templ)
{
    frameSize = cv::Size(0,0);
    lastValid = cv::Rect();
}

FullTemplateLocator::~FullTemplateLocator()
{

}

cv::Rect FullTemplateLocator::run(
        const cv::Mat& input)
{
    if(input.size() != frameSize)
    {
        frameSize = input.size();
        float factor = frameSize.width/640.0 / scaleDown;
        cv::resize(templateImage, templateImage,
                   cv::Size(templateImage.size().width * factor,
                            templateImage.size().height * factor));

        lastValid = cv::Rect();
    }


    cv::Mat resized;
    cv::resize(input, resized, cv::Size(frameSize.width*1.0/scaleDown,
                                        frameSize.height*1.0/scaleDown),
               cv::INTER_AREA);

    cv::Mat matchResult;
    cv::matchTemplate(resized, templateImage, matchResult, CV_TM_SQDIFF_NORMED);

    cv::Point mm;
    double matchVal;
    cv::minMaxLoc(matchResult, &matchVal, 0, &mm, 0);

    bool valid = (matchVal < 0.1);

    if(valid)
    {
        cv::Rect newRec = cv::Rect(mm.x * scaleDown, mm.y * scaleDown,
                                   templateImage.size().width * scaleDown,
                                   templateImage.size().height * scaleDown);

        if(lastValid == cv::Rect())
            lastValid = newRec;
        else
        {
            lastValid.x = lastValid.x * 0.98 + newRec.x * 0.02;
            lastValid.y = lastValid.y * 0.98 + newRec.y * 0.02;
        }

        if(lastValid.x < 0)
            lastValid.x = 0;
        if(lastValid.y < 0)
            lastValid.y = 0;

        if(lastValid.br().x > input.size().width)
            lastValid.width = input.size().width - lastValid.x;
        if(lastValid.br().y > input.size().height)
            lastValid.height = input.size().height - lastValid.y;
    }

    return lastValid;
}

}
