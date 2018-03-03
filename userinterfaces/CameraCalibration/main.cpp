
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#include <cstddef>
#include <opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/legacy/compat.hpp>
#include <highgui/highgui.hpp>

#include <string>
#include <sstream>

using namespace cv;

int deviceNum = 0;

void setFocus(int value)
{
    std::stringstream s;
    s << "v4l2-ctl --set-ctrl=focus_absolute=";
    s << value;
    s << " --device=";
    s << deviceNum;
    system(s.str().c_str());
}

void drawUI(cv::Mat& m, int phase)
{
    cv::rectangle(m, Rect(10, 70, 370, 220), (phase==2)? Scalar(0,255,0) : Scalar(0,0,255), 2, CV_AA);

    cv::Point mid(200, 190);

    switch(phase)
    {
    case 0:
        cv::line(m, mid - Point(10,0),  mid + Point(10,0), Scalar(0,0,255));
        cv::line(m, mid - Point(0,10),  mid + Point(0,10), Scalar(0,0,255));
    break;

    case 1:
        cv::rectangle(m, Rect(mid - Point(50,50), Size(100,100)), Scalar(0,255,255));
    break;

    case 2:
        cv::putText(m, "Calibration successful. Press 'R' to refocus.", mid + Point(-110, 85), CV_FONT_HERSHEY_PLAIN, 0.75, Scalar(0,0,0), 1, CV_AA);
    break;
    }

}


int main(int argc, char *argv[])
{
    cv::Size frameSize(1280,800);    
    if(argc == 2)
        deviceNum = atoi(argv[1]);

    cv::VideoCapture device(deviceNum);
    device.set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
    device.set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);

    if(!device.isOpened())
        return -1;

    std::stringstream s;
    s << "v4l2-ctl --set-ctrl=focus_auto=0";
    s << " --device=";
    s << deviceNum;
    system(s.str().c_str());

    int focus = 15;
    long maximum = 0;
    int bestFocus = 0;
    int wait = 0;

    setFocus(focus);
    while(cv::waitKey(5) == -1)
    {
        Mat rawImage, grayImage;
        device >> rawImage;
        if(rawImage.empty()) break;
        if(rawImage.size() != frameSize) break;
        cvtColor(rawImage, grayImage, CV_BGR2GRAY);
        transpose(grayImage, grayImage);
        flip(grayImage, grayImage, 0);

        Mat smaller;
        cvtColor(grayImage, smaller, CV_GRAY2RGB);
        resize(smaller, smaller, cv::Size(frameSize.height/2.0, frameSize.width/2.0));
        drawUI(smaller, 0);
        cv::imshow("Camera", smaller);
    }

    retry:

    maximum = 0;
    bestFocus = 0;
    focus = 15;
    long total = 0;
    while(focus <= 40)
    {        
        Mat rawImage, grayImage;
        setFocus(focus);
        device >> rawImage;
        if(rawImage.empty()) break;
        if(rawImage.size() != frameSize) break;
        cvtColor(rawImage, grayImage, CV_BGR2GRAY);
        transpose(grayImage, grayImage);
        flip(grayImage, grayImage, 0);

        Mat analyze;
        cv::GaussianBlur(grayImage, analyze, cv::Size(9, 9), 1);
        cv::absdiff(grayImage, analyze, analyze);

        for(int x = (200 - 50)*2; x < (200 + 50)*2; x++)
            for(int y = (190 - 50)*2; y < (190 + 50)*2; y++)
            {
                uchar val = analyze.at<uchar>(x,y);
                if(val == 2)
                    total++;
            }

        if(total > maximum)
        {
            maximum = total;
            bestFocus = focus;
        }

        if(wait == 3)
        {
            focus++;
            total = 0;
            wait = 0;
        }

        wait++;

        Mat smaller;
        cvtColor(grayImage, smaller, CV_GRAY2RGB);
        resize(smaller, smaller, cv::Size(frameSize.height/2.0, frameSize.width/2.0));

        drawUI(smaller, 1);
        cv::imshow("Camera", smaller);
        cv::waitKey(1);
    }

    setFocus(bestFocus);
    std::cout << bestFocus << std::endl;

    int code = -1;
    while(code == -1)
    {
        Mat rawImage, grayImage;
        device >> rawImage;
        if(rawImage.empty()) break;
        if(rawImage.size() != frameSize) break;
        cvtColor(rawImage, grayImage, CV_BGR2GRAY);
        transpose(grayImage, grayImage);
        flip(grayImage, grayImage, 0);

        Mat smaller;
        cvtColor(grayImage, smaller, CV_GRAY2RGB);
        resize(smaller, smaller, cv::Size(frameSize.height/2.0, frameSize.width/2.0));
        drawUI(smaller, 2);
        cv::imshow("Camera", smaller);
        code = cv::waitKey(5);
    }

    if(code == 1048690)
        goto retry;

    return 0;
}
