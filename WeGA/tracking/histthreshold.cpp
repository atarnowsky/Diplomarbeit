#include "histthreshold.h"

namespace WeGA {

HistThreshold::HistThreshold(
    scalar relArea,
    scalar scale) :
    relArea(relArea), scale(scale)
{
}

HistThreshold::~HistThreshold()
{

}

scalar HistThreshold::run(
    const cv::Mat& input,
    cv::Mat& output)
{
    scalar dX = (input.size().width - input.size().width * scale) * 0.5;
    scalar dY = (input.size().height - input.size().height * scale) * 0.5;
    cv::Mat scaledMat(input, cv::Rect(dX, dY, input.size().width * scale, input.size().height * scale));

    int bins = 255;
    int histSize[] = {bins};
    float range[] = { 0, 256 };
    const float* ranges[] = { range };
    cv::MatND hist;
    int channels[] = {0};
    float newArea = (relArea < 0.0)? 1.0 + relArea : relArea;

    cv::calcHist(&scaledMat, 1, channels, cv::Mat(),
                 hist, 1, histSize, ranges, true, false);

    cv::GaussianBlur(hist, hist, cv::Size(0,0), 1.0, 3.0);
    scalar area = scaledMat.size().area();

    int thresh = 0;
    scalar integral = 0.0;

    for(int n = 0; n < bins; n++)
    {
        float binVal = hist.at<float>(n)/area;
        integral += binVal;
        if(integral >= newArea)
        {
            thresh = n;
            break;
        }
    }

    cv::threshold(input, output, thresh, 255, (relArea > 0.0)? cv::THRESH_BINARY_INV : cv::THRESH_BINARY);

    return thresh;
}

}
