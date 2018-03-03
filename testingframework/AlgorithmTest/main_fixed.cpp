
#include <cstddef>
#include "WeGA/wega.h"
#include "WeGA/videoreader.h"
#include "WeGA/camerareader.h"
#include <sstream>

using namespace cv;


int main(int argc, char *argv[])
{
#ifdef VISUAL_MODE
    bool pausemode = false;
#endif
    std::string filename(argv[1]);
    char* size = argv[2];
    uchar pmeth = argv[3][0];
    uchar gmeth = argv[4][0];
    const char* thresh = "0.1";

    uchar pfil = 'F';
    uchar gfil = 'F';
    uchar prefil = 'F';

    if(argc > 5)
        pfil = argv[5][0];
    if(argc > 6)
        gfil = argv[6][0];
    if(argc > 7)
        prefil = argv[7][0];
    if(argc > 8)
        thresh = argv[8];

    int width = atoi(size);
    cv::Size frameSize(1280, 800);
    if(width > 0)
        frameSize = cv::Size(width, width * 800 / 1280);

    WeGA::FixedHeadTracker::ExtractionMethod pupilMethod = WeGA::FixedHeadTracker::BoundingBox;
    switch(pmeth)
    {
    case 'B':
        pupilMethod = WeGA::FixedHeadTracker::BoundingBox;
        break;
    case 'C':
        pupilMethod = WeGA::FixedHeadTracker::Centroid;
        break;
    case 'L':
        pupilMethod = WeGA::FixedHeadTracker::LeastSquares;
        break;
    case 'M':
        pupilMethod = WeGA::FixedHeadTracker::ModifiedLeastSquares;
        break;
    case 'O':
        pupilMethod = WeGA::FixedHeadTracker::OptimizedLSq;
        break;
    }

    WeGA::FixedHeadTracker::ExtractionMethod glintMethod = WeGA::FixedHeadTracker::BoundingBox;
    switch(gmeth)
    {
    case 'B':
        glintMethod = WeGA::FixedHeadTracker::BoundingBox;
        break;
    case 'C':
        glintMethod = WeGA::FixedHeadTracker::Centroid;
        break;
    case 'L':
        glintMethod = WeGA::FixedHeadTracker::LeastSquares;
        break;
    case 'M':
        glintMethod = WeGA::FixedHeadTracker::ModifiedLeastSquares;
        break;
    }

    int filters = 0;
    if(pfil == 'T')
        filters += WeGA::FixedHeadTracker::FilterPupil;
    if(gfil == 'T')
        filters += WeGA::FixedHeadTracker::FilterGlint;
    if(prefil == 'T')
        filters += WeGA::FixedHeadTracker::FilterBilateral;

    WeGA::FixedHeadTracker::TrackingTunables tunables;
    tunables.blurDiameter = 9.0;
    tunables.blurSigma = 5.0;
    tunables.pupilSize = atof(thresh);
    tunables.glintSize= -0.02;

    WeGA::FixedHeadTracker::DebugInfo info;

    WeGA::VideoReader reader(filename);
    //WeGA::CameraReader reader(0);
#ifdef VISUAL_MODE
    WeGA::FixedHeadTracker tracker(
                pupilMethod, glintMethod, filters,
                "EyeTemplate_Opened.png", tunables,
                &info);
#else
    WeGA::FixedHeadTracker tracker(
                pupilMethod, glintMethod, filters,
                "EyeTemplate_Opened.png", tunables);
#endif

    unsigned int frameNum = 0;


    while(reader.hasNext())
    {
        cv::Mat inputImage = reader.nextFrame();
        if(inputImage.size().area() <= 0) break;
        if(frameSize.width != inputImage.size().width)
            cv::resize(inputImage, inputImage, frameSize, 0, 0, cv::INTER_AREA);
        WeGA::TrackingResult result = tracker.analyzeFrame(inputImage);
        result.pupilGlintVector();

#ifdef VISUAL_MODE
        {
            cv::Mat inputImageCopy;
            cvtColor(inputImage, inputImageCopy, CV_GRAY2RGB);
            cv::transpose(inputImageCopy, inputImageCopy);
            cv::flip(inputImageCopy, inputImageCopy, 0);            

            cv::Mat overlayInv(inputImageCopy.size(), CV_8UC3);
            overlayInv.setTo(64);
            cv::rectangle(overlayInv, info.eyeRegion, CV_RGB(0,0,0), CV_FILLED);
            inputImageCopy -= overlayInv;

            cv::rectangle(inputImageCopy, info.eyeRegion, CV_RGB(0,128,128), 3, CV_AA);
            cv::rectangle(inputImageCopy, info.eyeRegion, CV_RGB(0,255,255), 1, CV_AA);

            cv::ellipse(inputImageCopy, info.pupilEllipse, CV_RGB(255,130,12), 2.5, CV_AA);
            imshow("Tracking result", inputImageCopy);
            std::cout << "Frame: " << frameNum << std::endl;
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
        frameNum++;

#ifdef LIST_MODE
        std::cout << frameNum++ << ";"
                  << result.toString() << "\n";
#endif
    }

#ifdef LIST_MODE
        std::cout.flush();
#endif
        std::cout << "# Frames processed: " << frameNum << std::endl;
    return 0;
}
