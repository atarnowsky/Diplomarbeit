#include "optimizedleastsquares.h"
#include <vector>
#include <map>
#include "momentummidpointextractor.h"

inline scalar distance(const vector2& a, const vector2& b)
{
    vector2 d = a - b;
    return sqrt(d.x*d.x+d.y*d.y);
}

inline scalar length(const vector2& a)
{
    return sqrt(a.x*a.x+a.y*a.y);
}

inline vector2 normalized(const vector2& a)
{
    scalar l = sqrt(a.x*a.x+a.y*a.y);
    return vector2(a.x/l, a.y/l);
}

namespace WeGA {

ComplexLeastSquaresEllipseFitter::ComplexLeastSquaresEllipseFitter(
        const scalar& scaleFactor,
        const scalar& usedThresh) :
    scaleFactor(scaleFactor), usedThresh(usedThresh)
{
    lastResult = cv::RotatedRect();
}

cv::RotatedRect ComplexLeastSquaresEllipseFitter::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    const int samples = 5*scaleFactor;

    if(inputBinary.size().area() == 0)
        return lastResult;

    cv::Mat cannyMat;
    cv::Canny(inputGray, cannyMat, 300 * 1.5 / scaleFactor, 500 * 1.5 / scaleFactor, 5, true);

    cv::Mat temp = inputGray.clone();
    cv::Mat gray32(inputGray.size(), CV_32FC1);
    cv::convertScaleAbs(temp, gray32);

    std::vector< std::vector<cv::Point> > points;
    std::vector<cv::Point> allpoints;
    cv::findContours(cannyMat, points, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

    for(unsigned int n = 0; n < points.size(); n++)
        if(points[n].size() > 3)
            allpoints.insert(allpoints.end(), points[n].begin(), points[n].end());

    if(allpoints.size() < 5)
        return lastResult;

    vector2 mid;

    cv::Mat gradX, gradY;
    cv::Scharr(inputGray, gradX, CV_64F, 1, 0, 0.025);
    cv::Scharr(inputGray, gradY, CV_64F, 0, 1, 0.025);

    cv::Mat blurred = inputGray;

    std::vector<cv::Point2f> validPoints;
    std::vector<cv::Point2f> directions;
    for(unsigned int n = 0; n < allpoints.size(); n++)
    {
        validPoints.push_back(cv::Point2f(allpoints[n].x, allpoints[n].y));
        directions.push_back(cv::Point2f(gradX.at<double>(allpoints[n]), gradY.at<double>(allpoints[n])));
    }



    std::vector<cv::Point2f> validPoints2 = validPoints;
    std::vector<cv::Point2f> directions2 = directions;

    validPoints.clear();
    directions.clear();

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        vector2 point  = validPoints2[n];        
        vector2 direction = normalized(directions2[n]);

        scalar value = 0.0;
        scalar value2 = 0.0;
        for(int m = 0; m < samples; m++)
        {
            int x = point.x - direction.x *  (m );
            int y = point.y - direction.y *  (m );

            int x2 = point.x + direction.x * (m ) * 1.0;
            int y2 = point.y + direction.y * (m ) * 1.0;

            if(y < 0 || x < 0 || y2 < 0 || x2 < 0 || y >= blurred.rows || y2  >= blurred.rows || x >= blurred.cols || x2 >= blurred.cols)
                continue;

            value += blurred.at<uchar>(y, x);
            value2 += blurred.at<uchar>(y2, x2);
        }
        value /= samples;
        value2 /= samples;

        if((value2 - value) > 15)
        {
            validPoints.push_back(point);
            directions.push_back(directions2[n]);
        }
    }

    if(validPoints.size() < 5)
        return lastResult;

    lastResult = cv::fitEllipse(validPoints);
    mid = lastResult.center;


    validPoints2 = validPoints;
    directions2 = directions;

    validPoints.clear();
    directions.clear();

    scalar gradMagnitudeAvg = 0.0;

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        vector2 diff = normalized(vector2(validPoints2[n].x - mid.x, validPoints2[n].y - mid.y));
        vector2 grad = normalized(directions2[n]);
        scalar factor = diff.dot(grad);

        if(factor >= 0.9)
        {
            validPoints.push_back(validPoints2[n]);
            directions.push_back(directions2[n]);

            gradMagnitudeAvg += length(directions[n]);
        }
    }

    gradMagnitudeAvg /= validPoints.size();
    scalar gradMagnitudeStddev = 0.0;

    for(unsigned int n = 0; n < directions.size(); n++)
    {
        scalar v = (length(directions[n]) - gradMagnitudeAvg);
        gradMagnitudeStddev += v*v;
    }

    gradMagnitudeStddev = sqrt(1.0/(directions.size()-1)*gradMagnitudeStddev);


    validPoints2 = validPoints;
    directions2 = directions;

    validPoints.clear();
    directions.clear();

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        if(fabs(length(directions2[n]) - gradMagnitudeAvg) <  4.0*gradMagnitudeStddev)
        {
            validPoints.push_back(validPoints2[n]);
            directions.push_back(directions2[n]);
        }
    }

    if(validPoints.size() < 5)
        return lastResult;


    lastResult = cv::fitEllipse(validPoints);
    mid = lastResult.center;

    validPoints2 = validPoints;
    directions2 = directions;

    validPoints.clear();
    directions.clear();

    scalar distanceAvg = 0.0;

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        distanceAvg += length(vector2(validPoints2[n].x - mid.x, validPoints2[n].y - mid.y));
    }

    distanceAvg /= validPoints2.size();
    scalar distanceStddev = 0.0;

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        scalar v = (length(vector2(validPoints2[n].x - mid.x, validPoints2[n].y - mid.y)) - distanceAvg);
        distanceStddev += v*v;
    }

    distanceStddev = sqrt(1.0/(validPoints2.size()-1)*distanceStddev);

    for(unsigned int n = 0; n < validPoints2.size(); n++)
    {
        if(fabs(length(vector2(validPoints2[n].x - mid.x, validPoints2[n].y - mid.y)) - distanceAvg) <  2.0*distanceStddev)
        {
            validPoints.push_back(validPoints2[n]);
        }
    }

    if(validPoints.size() < 5)
        return lastResult;

    lastResult = cv::fitEllipse(validPoints);
    return lastResult;
}

}
