#include "defaultcornerextractor.h"
#include "cvblobs/BlobResult.h"
#include "opencv2/highgui/highgui.hpp"

namespace WeGA {


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

inline float dist(vector2 p, cv::Size r)
{
    float dx = p.x - (r.width*0.5);
    float dy = p.y - (r.height*0.5);
    return sqrt(dx*dx+dy*dy);
}

inline float dist(vector2 p, vector2  r)
{
    float dx = p.x - r.x;
    float dy = p.y - r.y;
    return sqrt(dx*dx+dy*dy);
}

inline bool isInEllipse(const cv::RotatedRect& e, vector2& p)
{
    float b = 0;//e.center.x;
    float a = 0;//e.center.y;
    float shortAxis = e.size.width*0.5;
    float longAxis = e.size.height*0.5;
    float angle = -e.angle/180.0*M_PI;

    double distance = sqrt(longAxis*longAxis-shortAxis*shortAxis);

    vector2 f1(b+sin(angle)*distance, a+cos(angle)*distance);
    vector2 f2(b-sin(angle)*distance, a-cos(angle)*distance);

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

std::vector<vector2> DefaultCornerExtractor::run(
    const cv::Mat& inputBinary,
    const cv::Mat& inputGray)
{
    CBlobResult labels;
    IplImage labelImage(inputBinary);
    labels = CBlobResult(&labelImage, 0, 0);

    std::vector<vector2> result;

#ifdef DEBUG
    IplImage* debugOut = cvCreateImage(inputBinary.size(), 8, 3);
    cv::Mat debugImg(debugOut);
    cv::rectangle(debugImg,
                  cv::Rect(0,0,inputBinary.size().width,inputBinary.size().height),
                  CV_RGB(0,0,0),
                  CV_FILLED);

    drawBlobs(labels, debugOut, CV_RGB(255, 0,0));
#endif

    labels.Filter(labels, B_EXCLUDE, CBlobGetArea(),
                  B_GREATER, 20 * scaleFactor*scaleFactor);


#ifdef DEBUG
    drawBlobs(labels, debugOut, CV_RGB(0, 255,0));
    cv::Mat debugMat(debugOut);
#endif

    if(labels.GetNumBlobs() > 0)
    {
        CBlobGetXCenter xC;
        CBlobGetYCenter yC;

        for(int n = 0; n < labels.GetNumBlobs(); n++)
        {
            CBlob* blob = labels.GetBlob(n);
            vector2 g(xC(*blob), yC(*blob));

            int boundSize = 4*scaleFactor;

            cv::Rect bound(g.x - boundSize, g.y - boundSize, 2*boundSize, 2*boundSize);
            if(bound.tl().x < 0 || bound.tl().y < 0 || bound.br().x > inputGray.cols || bound.br().y > inputGray.rows)
                continue;
#ifdef DEBUG
            cv::rectangle(debugMat, bound, CV_RGB(0,255,255));
#endif

            cv::Mat bounds(inputGray, bound);
            cv::Moments m = cv::moments(bounds);
            vector2 centroid(m.m10/m.m00, m.m01/m.m00);
            result.push_back(centroid + vector2(bound.tl().x, bound.tl().y));
        }
    }

#ifdef DEBUG    
    cv::imshow("Corners debug out", debugMat);
#endif

    return result;
}

}
