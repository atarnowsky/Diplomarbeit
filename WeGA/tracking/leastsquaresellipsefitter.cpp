#include "leastsquaresellipsefitter.h"
#include <vector>

namespace WeGA {

LeastSquaresEllipseFitter::LeastSquaresEllipseFitter()
{

}

LeastSquaresEllipseFitter::~LeastSquaresEllipseFitter()
{

}

cv::RotatedRect LeastSquaresEllipseFitter::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    if(inputBinary.size().area() == 0)
        return cv::RotatedRect();

    cv::Mat temp = inputGray.clone();
    cv::Mat gray32(inputGray.size(), CV_32FC1);
    cv::convertScaleAbs(temp, gray32);

    cv::Mat lapMat;
    cv::Laplacian(gray32, lapMat, CV_32FC1, 1);
    cv::GaussianBlur(lapMat,lapMat,cv::Size(9,9),2.0);

    cv::Mat lapMat2(inputGray.size(), CV_8UC1);
    lapMat2.setTo(0);

    //Search for zero crossings
    const int wSize = 3;
    const int wSize2 = (wSize-1)/2;
    const int thresh = 0;
    const int thresh2 = 0;

    for(int x = wSize2; x < lapMat.rows - wSize2; x++)
        for(int y = wSize2; y < lapMat.cols - wSize2; y++)
        {
            float currentVal = lapMat.at<float>(x, y);
            if(fabs(currentVal) < thresh2) continue;
            int currentSign = (currentVal >= 0)? 1 : -1;
            bool found = false;
            if(currentSign > 0) continue;

            for(int a = -wSize2; a <= wSize2; a++)
                for(int b = -wSize2; b <= wSize2; b++)
                {
                    if(found) continue;
                    float secondVal = lapMat.at<float>(x+a, y+b);
                    int secondSign = (secondVal >= 0)? 1 : -1;
                    if(secondSign != currentSign)
                    {
                        if(fabs(secondVal - currentVal) > thresh)
                        {
                            lapMat2.at<uchar>(x, y) = 255;
                            found = true;
                        }
                    }
                }
        }

    std::vector<cv::Point2f> points;
    points.clear();

    for(int x = 0; x < lapMat2.rows; x++)
        for(int y = 0; y < lapMat2.cols; y++)
            if(lapMat2.at<uchar>(x, y) > 0)
                points.push_back(cv::Point2f(y, x));

    if(points.size() < 5)
        return cv::RotatedRect();

    cv::Mat mask = inputBinary.clone();
    cv::dilate(mask, mask, cv::Mat(), cv::Point(-1,-1), 3);
    cv::bitwise_and(mask, lapMat2, lapMat2);

    // This loop will only work on single-channel images:
    assert(lapMat2.type() == CV_8UC1);

    points.clear();

    for(int x = 0; x < lapMat2.rows; x++)
        for(int y = 0; y < lapMat2.cols; y++)
            if(lapMat2.at<uchar>(x, y) > 0)
                points.push_back(cv::Point2f(y, x));

    if(points.size() < 5)
        return cv::RotatedRect();

    return cv::fitEllipse(points);
}

}
