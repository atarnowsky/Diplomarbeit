#include "defaultpupilselector.h"
#include "cvblobs/BlobResult.h"
#include "cvblobs/BlobContour.h"

namespace WeGA {

DefaultPupilSelector::DefaultPupilSelector(
    const scalar& sizeFactor) :
    lastPupilPosition(cv::Point(-1,0)),
    sizeFactor(sizeFactor)
{
    avgFill = 0.785;
    avgArea = 0;
}

DefaultPupilSelector::~DefaultPupilSelector()
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

cv::Rect DefaultPupilSelector::run(
    const cv::Mat& input,
    const cv::Mat& gray,
    cv::Mat& region)
{
    int safetyMargin = 15;
    if(input.size().width < 640)
        safetyMargin = 7;

    //bool reinitPosition = (lastPupilPosition.x == -1);
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
    drawBlobs(labels, debugOut, CV_RGB(255, 0,0));
#endif

    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_LESS, 50 * sizeFactor*sizeFactor);

   /* labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_GREATER, 5500 * sizeFactor*sizeFactor);*/
   /* labels.Filter(labels, B_EXCLUDE, CBlobGetCompactness(),
                  B_GREATER, 3.5);*/

#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(255, 128,0));
#endif

    labels.Filter(labels, B_EXCLUDE, CBlobGetMoment(0,0),
                  B_LESS, 0.75*avgArea);
/*
    if(!reinitPosition)
    {
        labels.Filter(labels, B_EXCLUDE,
                      CBlobGetDistanceFromPoint(lastPupilPosition.x,
                                                lastPupilPosition.y),
                      B_GREATER, 150 * sizeFactor);
    }
*/
#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(255, 255,0));
#endif

    double darkest = 255.0;
    scalar bestFill = 0.0;
    CBlob* bestBlob = 0;
    if(labels.GetNumBlobs() > 0)
    {        
        IplImage iplGray(gray);
        for(int n = 0; n < labels.GetNumBlobs(); n++)
        {
            CBlob* blob = labels.GetBlob(n);

            scalar fillCurrent = blob->Moment(0,0)/(blob->GetBoundingBox().width*blob->GetBoundingBox().height);

            double mean = blob->Mean(&iplGray);
//std::cout << mean << ", " << avgDarkness << std::endl;
            //if(fabs(mean-avgDarkness) < fabs(darkest - avgDarkness))
            {
                darkest = mean;
                cv::Rect r(labels.GetBlob(n)->MinX(), labels.GetBlob(n)->MinY(),
                       labels.GetBlob(n)->MaxX() - labels.GetBlob(n)->MinX(),
                       labels.GetBlob(n)->MaxY() - labels.GetBlob(n)->MinY());
                r.x -= safetyMargin; r.y -= safetyMargin;
                r.width += safetyMargin * 2; r.height += safetyMargin * 2;

                if(r.x < 0) r.x = 0;
                if(r.y < 0) r.y = 0;

                if(r.x + r.width > input.size().width) continue;
                if(r.y + r.height > input.size().height) continue;

                if(fabs(avgFill - fillCurrent) > 0.25)
                    continue;

                if(fabs(avgFill - fillCurrent) <= fabs(avgFill - bestFill))
                {
                    result = r;
                    bestFill = fillCurrent;
                    bestBlob = blob;
                }
            }
        }
    }
    avgArea = avgArea * 0.8;

    if(result.area() > 0)
    {
        lastPupilPosition = cv::Point(result.x + result.width/2, result.y + result.height/2);
        IplImage* fillImage = cvCreateImage(input.size(), 8, 1);
        cv::Mat fillMat(fillImage);
        fillMat.setTo(0);
        bestBlob->FillBlob(fillImage, CV_RGB(255,255,255));         
        region = cv::Mat(fillMat, result).clone();
        cvReleaseImage(&fillImage);
        avgDarkness = avgDarkness * 0.9 + darkest * 0.1;
        avgFill = avgFill * 0.9 + bestFill * 0.1;
        avgArea += bestBlob->Moment(0,0) * 0.2;
    }
    else
        lastPupilPosition = cv::Point(-1, -1);


#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(0,255,0));

    cv::Mat debugMat(debugOut);
    cv::imshow("Pupil debug out", debugMat);
#endif

    return result;
}

}
