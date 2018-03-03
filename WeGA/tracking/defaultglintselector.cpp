#include "defaultglintselector.h"
#include "cvblobs/BlobResult.h"
#include "opencv2/highgui/highgui.hpp"

namespace WeGA {

DefaultGlintSelector::DefaultGlintSelector(
    const scalar& sizeFactor,
    vector2& pupilPosition) :
    lastGlintPosition(cv::Point(-1,0)),
    sizeFactor(sizeFactor),
    pupil(pupilPosition)
{
}

DefaultGlintSelector::~DefaultGlintSelector()
{

}


inline void drawBlobs(CBlobResult& blobs, IplImage* canvas, CvScalar color)
{
    if(blobs.GetNumBlobs() > 0)
    {
        for(int n = 0; n < blobs.GetNumBlobs(); n++)
        {
            blobs.GetBlob(n)->FillBlob(canvas, color);
        }
    }
}

cv::Rect DefaultGlintSelector::run(
    const cv::Mat& input,
    const cv::Mat& gray,
    cv::Mat& region)
{
    const int safetyMargin = 5;

    //bool reinitPosition = (lastGlintPosition.x == -1);
    CBlobResult labels;

    IplImage labelImage(input);
    labels = CBlobResult(&labelImage, 0, 0);

    cv::Rect result(0, 0, 0, 0);

#ifdef DEBUG
    IplImage* debugOut = cvCreateImage(input.size(), 8, 3);
    cv::Mat debugImg(debugOut);
    cv::rectangle(debugImg,
                  cv::Rect(0,0,input.size().width,input.size().height),
                  CV_RGB(0,0,0),
                  CV_FILLED);
#endif

    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_LESS, 4 * sizeFactor*sizeFactor);

#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(255, 0,0));
#endif


    // Circumvents a strange cvBloblib bug, where the whole image becomes a single blob
    /*labels.Filter(labels, B_EXCLUDE, CBlobGetCompactness(),
                  B_GREATER, 35 * sizeFactor);*/

#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(255, 128,0));
#endif
/*


    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_GREATER, 35 * sizeFactor*sizeFactor);
*/
    labels.Filter(labels, B_EXCLUDE, CBlobGetElongation(),
                  B_GREATER, 3);
/*
    if(!reinitPosition)
        labels.Filter(labels, B_EXCLUDE,
                      CBlobGetDistanceFromPoint(lastGlintPosition.x,
                                                lastGlintPosition.y),
                      B_GREATER, 50 * sizeFactor);*/

#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(255, 255,0));
#endif

    // Position relative to pupil
    CBlob* bestBlob = 0;
    if(labels.GetNumBlobs() > 0)
    {
        CBlob* minDistanceBlob = labels.GetBlob(0);
        CBlobGetXCenter xC;
        CBlobGetYCenter yC;
        float dX = xC(*minDistanceBlob) - pupil.x;
        float dY = yC(*minDistanceBlob) - pupil.y;
        float distance = dX*dX + dY*dY;

        float minDistance = distance;

        for(int n = 1; n < labels.GetNumBlobs(); n++)
        {
            CBlob* blob = labels.GetBlob(n);
            float dX = xC(*blob) - pupil.x;
            float dY = yC(*blob) - pupil.y;
            float distance = dX*dX + dY*dY;

            if(distance < minDistance)
            {
                minDistanceBlob = blob;
                minDistance = distance;
            }
        }

        result = minDistanceBlob->GetBoundingBox();
        result.x -= safetyMargin; result.y -= safetyMargin;
        result.width += safetyMargin * 2; result.height += safetyMargin * 2;

        if(result.x < 0) result.x = 0;
        if(result.y < 0) result.y = 0;

        if(result.x + result.width > input.size().width)
            result = cv::Rect(0, 0, 0, 0);
        if(result.y + result.height > input.size().height)
            result = cv::Rect(0, 0, 0, 0);

        bestBlob = minDistanceBlob;
    }

    if(result.area() > 0)
    {
        lastGlintPosition = cv::Point(result.x + result.width/2, result.y + result.height/2);
        IplImage* fillImage = cvCreateImage(input.size(), 8, 1);
        cv::Mat fillMat(fillImage);
        fillMat.setTo(0);
        bestBlob->FillBlob(fillImage, CV_RGB(255,255,255));
        region = cv::Mat(fillMat, result).clone();
        cvReleaseImage(&fillImage);
    }
    else
        lastGlintPosition = cv::Point(-1, -1);

#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(0,255,0));

    cv::Mat debugMat(debugOut);
    cv::imshow("Glint debug out", debugMat);
#endif

    return result;
}

}
