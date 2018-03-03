#include "enhancedleastsquares.h"
#include <vector>

inline double distance(cv::Point2f a, cv::Point2f b)
{
    return sqrt((a.x - b.x)*(a.x - b.x)+(a.y - b.y)*(a.y - b.y));
}

namespace WeGA {

BinaryLeastSquaresEllipseFitter::BinaryLeastSquaresEllipseFitter(
        vector2& glintPosition, const scalar& scaleFactor) :
    glint(glintPosition), scaleFactor(scaleFactor) { }

cv::RotatedRect BinaryLeastSquaresEllipseFitter::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    if(inputBinary.size().area() == 0)
    {
        return cv::RotatedRect();
    }

    cv::Mat temp = inputBinary.clone();
    cv::Mat gray32(inputGray.size(), CV_32FC1);
    cv::convertScaleAbs(temp, gray32);

    cv::Mat lapMat2;
    cv::Laplacian(gray32, lapMat2, CV_32FC1, 1);

    cv::Mat lapMat(inputGray.size(), CV_8UC1);
    lapMat.setTo(0);

    //Search for zero crossings
    const int wSize = 3;
    const int wSize2 = (wSize-1)/2;

    for(int x = wSize2; x < lapMat2.rows - wSize2; x++)
        for(int y = wSize2; y < lapMat2.cols - wSize2; y++)
        {
            float currentVal = lapMat2.at<float>(x, y);
            int currentSign = (currentVal >= 0)? 1 : -1;
            bool found = false;
            if(currentSign > 0) continue;

            for(int a = -wSize2; a <= wSize2; a++)
                for(int b = -wSize2; b <= wSize2; b++)
                {
                    if(found) continue;
                    float secondVal = lapMat2.at<float>(x+a, y+b);
                    int secondSign = (secondVal >= 0)? 1 : -1;
                    if(secondSign != currentSign)
                    {
                            lapMat.at<uchar>(x, y) = 255;
                            found = true;
                    }
                }
        }

    std::vector< std::vector<cv::Point> > points;
    cv::findContours(lapMat, points, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    int maxIndex = 0;
    for(unsigned int n = 0; n < points.size(); n++)
        if(points[n].size() > points[maxIndex].size())
            maxIndex = n;

    if(points[maxIndex].size() < 5)
    {
        return cv::RotatedRect();
    }

    cv::Mat mask(inputGray.size(), CV_8UC1);
    mask.setTo(0);
    std::vector<cv::Point> validPoints;
    for(unsigned int n = 0; n < points[maxIndex].size(); n++)
    {
        if(distance(points[maxIndex][n], glint) > 15*scaleFactor)
        {
            validPoints.push_back(points[maxIndex][n]);
            mask.at<uchar>(points[maxIndex][n])   = 255;
        }
    }

    if(validPoints.size() < 5)
        return cv::RotatedRect();
    else
        return cv::fitEllipse(validPoints);

}

}
