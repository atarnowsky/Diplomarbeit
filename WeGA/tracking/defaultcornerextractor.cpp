#include "defaultcornerextractor.h"
#include "cvblobs/BlobResult.h"
#include "highgui/highgui.hpp"

inline int getBlobDirection(const cv::Mat& image,
                            CBlob& blob,
                            cv::Point2f& centroid)
{
    CBlobGetXCenter xC;
    CBlobGetYCenter yC;

    const int boundRadius = 16;

    cv::Point2f center(xC(blob), yC(blob));

    cv::Rect bbox(center.x - boundRadius, center.y - boundRadius,
                  boundRadius * 2, boundRadius * 2);

    if(bbox.tl().x < 0 || bbox.tl().y < 0 ||
            bbox.br().x > image.cols || bbox.br().y > image.rows)
        return -1;

    cv::Mat mIn(image, bbox);

    cv::Moments moments = cv::moments(mIn);
    centroid.x = moments.m10/moments.m00 + bbox.tl().x;
    centroid.y = moments.m01/moments.m00 + bbox.tl().y;

    int lightest = 0;
    cv::Point2f lPos;

    for(int x = 0; x < mIn.cols; x++)
        for(int y = 0; y < mIn.rows; y++)
            if(mIn.at<uchar>(x,y) >= lightest)
            {
                lightest = mIn.at<uchar>(x,y);
                lPos = cv::Point2f(x,y);
            }


    cv::Mat mOut;
    cv::Laplacian(mIn, mOut, 8);

    uchar thres = 1;
    const int stripeWidth = 2;

retry:

    // Left
    int maxLeft = 0;
    for(int y = lPos.y - stripeWidth; y <= lPos.y + stripeWidth; y++)
    {
        int longestDist = 0;
        for(int x = lPos.x; x >= 0; x--)
        {
            if(mOut.at<uchar>(x,y) >= thres)
                break;
            else
                longestDist++;
        }
        maxLeft += longestDist;
    }

    // Right
    int maxRight = 0;
    for(int y = lPos.y - stripeWidth; y <= lPos.y + stripeWidth; y++)
    {
        int longestDist = 0;
        for(int x = lPos.x; x < mOut.cols; x++)
        {
            if(mOut.at<uchar>(x,y) >= thres)
                break;
            else
                longestDist++;
        }
        maxRight += longestDist;
    }

    // Top
    int maxTop = 0;
    for(int x = lPos.x - stripeWidth; x <= lPos.x + stripeWidth; x++)
    {
        int longestDist = 0;
        for(int y = lPos.y; y >= 0; y--)
        {
            if(mOut.at<uchar>(x,y) >= thres)
                break;
            else
                longestDist++;
        }
        maxTop += longestDist;
    }

    // Bottom
    int maxBottom = 0;
    for(int x = lPos.x - stripeWidth; x <= lPos.x + stripeWidth; x++)
    {
        int longestDist = 0;
        for(int y = lPos.y; y < mOut.rows; y++)
        {
            if(mOut.at<uchar>(x,y) >= thres)
                break;
            else
                longestDist++;
        }
        maxBottom += longestDist;
    }

    int biggest = std::max(maxLeft, std::max(maxRight, std::max(maxTop, maxBottom)));

    int secondBiggest = 0;
    if(maxLeft > secondBiggest && maxLeft != biggest)
        secondBiggest = maxLeft;
    if(maxRight > secondBiggest && maxRight != biggest)
        secondBiggest = maxRight;
    if(maxTop > secondBiggest && maxTop != biggest)
        secondBiggest = maxTop;
    if(maxBottom > secondBiggest && maxBottom != biggest)
        secondBiggest = maxBottom;

#ifdef DEBUG
    std::cout << biggest - secondBiggest << " -> ";
#endif

    if(biggest - secondBiggest < 5 && thres < 32)
    {
        thres++;
        goto retry;
    }

#ifdef DEBUG
    std::cout << "[" << maxLeft << ", " << maxRight << "," << maxTop << ", " << maxBottom << "]" << std::endl;
#endif

    if(biggest == maxRight)
        return 2;
    else if(biggest == maxLeft)
        return 1;
    else if(biggest == maxTop)
        return 3;
    else
        return 0;
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

inline float dist(cv::Point2f p, cv::Size r)
{
    float dx = p.x - (r.width*0.5);
    float dy = p.y - (r.height*0.5);
    return sqrt(dx*dx+dy*dy);
}

inline float dist(cv::Point2f p, cv::Point2f  r)
{
    float dx = p.x - r.x;
    float dy = p.y - r.y;
    return sqrt(dx*dx+dy*dy);
}

inline bool isInEllipse(const cv::RotatedRect& e, cv::Point2f& p)
{
    float b = 0;//e.center.x;
    float a = 0;//e.center.y;
    float shortAxis = e.size.width*0.5;
    float longAxis = e.size.height*0.5;
    float angle = -e.angle/180.0*M_PI;

    double distance = sqrt(longAxis*longAxis-shortAxis*shortAxis);

    cv::Point2f f1(b+sin(angle)*distance, a+cos(angle)*distance);
    cv::Point2f f2(b-sin(angle)*distance, a-cos(angle)*distance);

    float d = 2.0*longAxis;

    float d2 = dist(p, f1) + dist(p, f2);

    //std::cout << a << ", " << b << " ; " << p.x << ", " << p.y << " / " << d << " - " << d2 << std::endl;

    return (d2 <= d);
}

DefaultCornerExtractor::DefaultCornerExtractor(
    float scaleFactor) :
    scaleFactor(scaleFactor)
{
}

CornerSet DefaultCornerExtractor::operator() (
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray,
    const cv::RotatedRect& ellipse,
    float& confidence)
{
    CBlobResult labels;
    IplImage labelImage(inputBinary);
    labels = CBlobResult(&labelImage, 0, 0);

    CornerSet result;
    for(int n = 0; n < 4; n++)
        result.status[n] = -1;


#ifdef DEBUG
    IplImage* debugOut = cvCreateImage(inputBinary.size(), 8, 3);
    cv::Mat debugImg(debugOut);
    cv::rectangle(debugImg,
                  cv::Rect(0,0,inputBinary.size().width,inputBinary.size().height),
                  CV_RGB(0,0,0),
                  CV_FILLED);

    drawBlobs(labels, debugOut, CV_RGB(255, 0,0));
#endif
/*
    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_LESS, 2*scaleFactor*scaleFactor);*/

    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_GREATER, 20 * scaleFactor*scaleFactor);


#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(0, 255,0));
    cv::Mat debugMat(debugOut);
    //cv::Sobel(inputGray, debugMat, 8, 1, 0);

#endif

    if(labels.GetNumBlobs() > 0)
    {
        CBlobGetXCenter xC;
        CBlobGetYCenter yC;

        std::vector<cv::Point2f> candidates[4];

        for(int n = 0; n < labels.GetNumBlobs(); n++)
        {
            CBlob* blob = labels.GetBlob(n);
            float dX = inputBinary.size().width*0.5 - xC(*blob);
            float dY = inputBinary.size().height*0.5 - yC(*blob);

            cv::Point2f p(dX, dY);
            if(!isInEllipse(ellipse, p))
                continue;

            cv::Point2f g;
            int identifier = getBlobDirection(inputGray, *blob, g);

            if(identifier == -1)
                continue;

            candidates[identifier].push_back(g);

#ifdef DEBUG            
            cv::rectangle(debugMat, cv::Rect(g.x-8,g.y-8,16,16), CV_RGB(255,255,255));
#endif
        }

        // Extract topLeft point
        if(!candidates[0].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[0].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[0].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[0] = *p;
            result.status[0] = 0;

            candidates[0].erase(p);
        }
        else
            result.status[0] = -1;

        // Extract topRight point
        if(!candidates[1].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[1].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[1].end(); ++iter)
            {
                if(iter->x > p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[1] = *p;
            result.status[1] = 0;

            candidates[1].erase(p);
        }
        else
            result.status[1] = -1;

        // Extract bottomLeft point
        if(!candidates[2].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[2].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[2].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y > p->y)
                    p = iter;
            }

            result.point[2] = *p;
            result.status[2] = 0;

            candidates[2].erase(p);
        }
        else
            result.status[2] = -1;

        // Extract bottomRight point
        if(!candidates[3].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[3].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[3].end(); ++iter)
            {
                if(iter->x > p->x)
                    p = iter;
                if(iter->y > p->y)
                    p = iter;
            }

            result.point[3] = *p;
            result.status[3] = 0;

            candidates[3].erase(p);
        }
        else
            result.status[3] = -1;


        // Nothing found for topLeft? Maybe there is a misidentified one left in bottomRight?
        if(result.status[0] == -1 && !candidates[3].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[3].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[3].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[0] = *p;
            result.status[0] = 0;
            candidates[3].erase(p);
        }

        // Nothing found for bottomRight? Maybe there is a misidentified one left in topLeft?
        if(result.status[3] == -1 && !candidates[0].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[0].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[0].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[3] = *p;
            result.status[3] = 0;
            candidates[0].erase(p);
        }

        // Nothing found for topRight? Maybe there is a misidentified one left in bottomLeft?
        if(result.status[1] == -1 && !candidates[2].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[2].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[2].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[1] = *p;
            result.status[1] = 0;
            candidates[2].erase(p);
        }

        // Nothing found for bottomLeft? Maybe there is a misidentified one left in topRight?
        if(result.status[2] == -1 && !candidates[1].empty())
        {
            std::vector<cv::Point2f>::iterator iter = candidates[1].begin();
            std::vector<cv::Point2f>::iterator p = iter;
            for(; iter != candidates[1].end(); ++iter)
            {
                if(iter->x < p->x)
                    p = iter;
                if(iter->y < p->y)
                    p = iter;
            }

            result.point[2] = *p;
            result.status[2] = 0;
            candidates[1].erase(p);
        }

        // Distribute remaining points
        for(int n = 0; n < 4; n++)
            if(result.status[n] == -1)
            {
                for(int m = 0; m < 4; m++)
                    if(candidates[m].size() > 0)
                    {
                        result.point[n] = candidates[m][candidates[m].size()-1];
                        result.status[n] = 0;
                        candidates[m].pop_back();
                        break;
                    }
            }

        // Remove duplicates
        for(int n = 0; n < 4; n++)
            for(int m = n; m < 4; m++)
                if(m != n && dist(result.point[m], result.point[n]) < 10.0)
                    result.status[m] = -1;

        // Additional tests:
        if((result.status[0] + result.status[3]) == 0)
            if(result.point[0].y > result.point[3].y)
                std::swap(result.point[0], result.point[3]);

        if((result.status[1] + result.status[2]) == 0)
            if(result.point[1].y > result.point[2].y)
                std::swap(result.point[1], result.point[2]);

        if((result.status[1] + result.status[2]) == 0)
            if(result.point[2].y - result.point[1].y < 10)
            {
                std::swap(result.point[0], result.point[2]);
                std::swap(result.status[0], result.status[2]);
            }


        if((result.status[0] + result.status[1] + result.status[2]) == 0)
            if(result.point[1].x > result.point[0].x &&
                    result.point[1].x > result.point[2].x)
                if(result.point[0].y > result.point[2].y)
                    std::swap(result.point[0], result.point[2]);


        //TODO: Write many many more cases...

#ifdef DEBUG
        std::cout << std::endl;
#endif
    }

#ifdef DEBUG    
    cv::imshow("Corners debug out", debugMat);
#endif

    confidence = 0;

    return result;
}
