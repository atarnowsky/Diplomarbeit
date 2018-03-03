
#include <cstddef>
#include "WeGA/wega.h"
#include "WeGA/videoreader.h"
#include <sstream>

using namespace cv;
using namespace WeGA;

scalar sizeFactor;


inline vector2 normalized(const vector2& a)
{
    scalar l = sqrt(a.x*a.x+a.y*a.y);
    return vector2(a.x/l, a.y/l);
}

inline scalar length(const vector2& a)
{
    return sqrt(a.x*a.x+a.y*a.y);
}

inline void linePair(cv::Mat& image,
                     std::pair<vector2, TrackingResult::CornerStatus>& a,
                     std::pair<vector2, TrackingResult::CornerStatus>& b)
{
    const float l = 10.0;
    vector2 diff; scalar len;
    cv::Scalar color;
    if(a.second != TrackingResult::Missing && b.second != TrackingResult::Missing)
    {
        color = (a.second == TrackingResult::Interpolated)? CV_RGB(255,128,0) : CV_RGB(0,255,128);
        diff = b.first - a.first; len = sqrt(diff.x*diff.x+diff.y*diff.y); diff.x /= len; diff.y /= len;
        cv::line(image, a.first *16*sizeFactor, (a.first + diff * l) * 16*sizeFactor, color, 1, CV_AA, 4);
        color = (b.second == TrackingResult::Interpolated)? CV_RGB(255,128,0) : CV_RGB(0,255,128);
        cv::line(image, b.first *16*sizeFactor, (b.first - diff * l) * 16*sizeFactor, color, 1, CV_AA, 4);
    }
}


inline void drawCross(cv::Mat& image, const WeGA::TrackingResult& r)
{
    std::pair<vector2, TrackingResult::CornerStatus> a = r.corner(WeGA::TrackingResult::TopLeft);
    std::pair<vector2, TrackingResult::CornerStatus> b = r.corner(WeGA::TrackingResult::TopRight);
    std::pair<vector2, TrackingResult::CornerStatus> c = r.corner(WeGA::TrackingResult::BottomLeft);
    std::pair<vector2, TrackingResult::CornerStatus> d = r.corner(WeGA::TrackingResult::BottomRight);

    linePair(image,a,b);
    linePair(image,a,c);
    linePair(image,d,b);
    linePair(image,d,c);
}


int main(int argc, char *argv[])
{
#ifdef VISUAL_MODE
    bool pausemode = false;
#endif
    std::string filename(argv[1]);
    char* size = argv[2];
    int width = atoi(size);
    cv::Size frameSize(1280, 800);
    if(width > 0)
        frameSize = cv::Size(width, width * 800 / 1280);

    WeGA::FixedHeadTracker::ExtractionMethod pupilMethod = WeGA::FixedHeadTracker::OptimizedLSq;
    WeGA::FixedHeadTracker::ExtractionMethod glintMethod = WeGA::FixedHeadTracker::Centroid;
    WeGA::FixedHeadTracker::TrackingTunables tunables;
    tunables.pupilSize = 0.04;
    tunables.glintSize= -0.02;

    WeGA::FixedHeadTracker::DebugInfo info;

    WeGA::VideoReader reader(filename);
#ifdef VISUAL_MODE
    WeGA::InvariantHeadTracker tracker(
                pupilMethod, glintMethod, 0,
                "EyeTemplate_Opened.png", tunables,
                &info);
#else
    WeGA::InvariantHeadTracker tracker(
                pupilMethod, glintMethod, 0,
                "EyeTemplate_Opened.png", tunables);
#endif

#ifdef LIST_MODE
    unsigned int frameNum = 0;
#endif
int frameNumber = 0;
    while(reader.hasNext())
    {
        frameNumber++;
        cv::Mat inputImage = reader.nextFrame();
        sizeFactor = inputImage.size().width/640.0;
        if(inputImage.size().area() <= 0) break;
        if(frameSize.width != 1280)
            cv::resize(inputImage, inputImage, frameSize, 0, 0, cv::INTER_AREA);
        WeGA::TrackingResult result = tracker.analyzeFrame(inputImage);
        result.pupilGlintVector();

#ifdef VISUAL_MODE
        {
            bool found = result.corner(TrackingResult::TopLeft).second != TrackingResult::Missing;
            cv::Mat inputImageCopy;
            cvtColor(inputImage, inputImageCopy, CV_GRAY2RGB);
            cv::transpose(inputImageCopy, inputImageCopy);
            cv::flip(inputImageCopy, inputImageCopy, 0);
            cv::Mat overlay(inputImageCopy.size(), CV_8UC3);
            overlay.setTo(0);

            cv::Mat overlayInv(inputImageCopy.size(), CV_8UC3);
            overlayInv.setTo(64);
            cv::rectangle(overlayInv, info.eyeRegion, CV_RGB(0,0,0), CV_FILLED);
            inputImageCopy -= overlayInv;

            cv::rectangle(inputImageCopy, info.eyeRegion, found? CV_RGB(0,128,128) : CV_RGB(128,64,0), 3, CV_AA);
            cv::rectangle(inputImageCopy, info.eyeRegion, found? CV_RGB(0,255,255) : CV_RGB(255,128,0), 1, CV_AA);

            if(found)
            {
                cv::Point pts[4] = {
                    cv::Point(result.corner(TrackingResult::TopRight).first.x * 16 * sizeFactor,
                              result.corner(TrackingResult::TopRight).first.y * 16 * sizeFactor),
                    cv::Point(result.corner(TrackingResult::TopLeft).first.x * 16 * sizeFactor,
                              result.corner(TrackingResult::TopLeft).first.y * 16 * sizeFactor),
                    cv::Point(result.corner(TrackingResult::BottomLeft).first.x * 16 * sizeFactor,
                              result.corner(TrackingResult::BottomLeft).first.y * 16 * sizeFactor),
                    cv::Point(result.corner(TrackingResult::BottomRight).first.x * 16 * sizeFactor,
                              result.corner(TrackingResult::BottomRight).first.y * 16 * sizeFactor)
                            };

                cv::fillConvexPoly(overlay, pts, 4, CV_RGB(32,48,64), CV_AA, 4);

                inputImageCopy += overlay;
                overlay.setTo(0);

                cv::line(overlay, result.glintPosition()*16*sizeFactor,
                         (result.glintPosition() + result.cornerVector(TrackingResult::TopLeft))*16*sizeFactor, CV_RGB(64,64,32), 1, CV_AA, 4);
                cv::line(overlay, result.glintPosition()*16*sizeFactor,
                         (result.glintPosition() + result.cornerVector(TrackingResult::TopRight))*16*sizeFactor, CV_RGB(64,64,32), 1, CV_AA, 4);
                cv::line(overlay, result.glintPosition()*16*sizeFactor,
                         (result.glintPosition() + result.cornerVector(TrackingResult::BottomLeft))*16*sizeFactor, CV_RGB(64,64,32), 1, CV_AA, 4);
                cv::line(overlay, result.glintPosition()*16*sizeFactor,
                         (result.glintPosition() + result.cornerVector(TrackingResult::BottomRight))*16*sizeFactor, CV_RGB(64,64,32), 1, CV_AA, 4);


                inputImageCopy += overlay;
            }
                cv::ellipse(inputImageCopy, info.pupilEllipse, CV_RGB(255,255,0), 1, CV_AA);
                cv::ellipse(inputImageCopy, info.glintEllipse, CV_RGB(0,255,0), 1, CV_AA);
                info.pupilEllipse.size.height = info.pupilEllipse.size.width = 5;
                info.glintEllipse.size.height = info.glintEllipse.size.width = 5;
                cv::ellipse(inputImageCopy, info.pupilEllipse, CV_RGB(255,255,0), 1, CV_AA);
                cv::ellipse(inputImageCopy, info.glintEllipse, CV_RGB(0,255,0), 1, CV_AA);
                cv::line(inputImageCopy, result.pupilPosition()*16*sizeFactor, result.glintPosition()*16*sizeFactor, CV_RGB(255,0,0), 1, CV_AA, 4);


                drawCross(inputImageCopy, result);


            imshow("Tracking result", inputImageCopy);
            
            if((char) waitKey(4) == 'p')
                pausemode = true;
            while(pausemode)
            {
                char code = waitKey(0);
                if(code == ' ')
                    break;
                else if( code == 'p')
                    pausemode = false;
            }
        }
#endif

#ifdef LIST_MODE
        std::cout << frameNum++ << ";"
                  << result.toString() << std::endl;
#endif
    }

    return 0;
}
