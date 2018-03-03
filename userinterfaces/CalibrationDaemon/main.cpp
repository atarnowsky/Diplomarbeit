#include <opencv2/opencv.hpp>
#include <highgui/highgui.hpp>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

extern "C" {
#include <X11/Xlocale.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/record.h>
#include <X11/XKBlib.h>
}

XRecordContext  context;
Display*        display;
cv::VideoCapture cam;
bool running = true;
cv::VideoWriter* writer = 0;
std::ofstream fileWriter;
std::string fileName;

const char* filepath = "/tmp";

const float screenWidth = 1920;
const float screenHeight = 1080;
int frameNum;

typedef union {
    unsigned char		type;
    xEvent				event;
    xResourceReq		req;
    xGenericReply		reply;
    xError				error;
    xConnSetupPrefix	setup;
} XRecordDatum;

long long lastTimestamp = 0;

bool timeUp()
{
    struct timeval timestamp;
    gettimeofday(&timestamp,NULL);

    long long currentTimestamp = timestamp.tv_sec * 100 + timestamp.tv_usec / 10000;

    if(currentTimestamp - lastTimestamp > 25)
    {
        lastTimestamp = currentTimestamp;
        return true;
    }
    else
        return false;
}



void saveCurrentCamera(int posX, int posY, bool doStream)
{
    if(!running) return;
    cv::Mat cap, gray;
    cam >> cap;

    cv::cvtColor(cap, gray, CV_RGB2GRAY);
    cv::cvtColor(gray, gray, CV_GRAY2RGB);

    if(!writer)
    {
        struct timeval timestamp;
        gettimeofday(&timestamp,NULL);

        std::stringstream s;
        s << filepath;
        s << "/tracking_";
        s << (long)timestamp.tv_sec;
        s << ".avi";
        writer = new cv::VideoWriter(s.str(), CV_FOURCC('H','F','Y','U'),
                                     1, gray.size(), true);
        frameNum= 1;

        s << ".log";
        fileName = s.str();
    }

    if(!writer->isOpened())
        return;


    writer->write(gray);

    std::stringstream buf;

    if(!doStream)
        buf << "#";
    buf << frameNum << ";" << posX/screenWidth << ";" << posY / screenHeight << std::endl;
    fileWriter.open(fileName.c_str(), std::ios::app | std::ios::out);
    fileWriter << buf.str();
    fileWriter.flush();
    fileWriter.close();
    frameNum++;
}

void eventCallback(XPointer c, XRecordInterceptData* data)
{
    if(data->data == 0)
        return;

    XRecordDatum* datum = (XRecordDatum*) data->data;

    switch (datum->type) {
        case KeyPress:
            if(datum->event.u.u.detail == 96
                    && datum->event.u.keyButtonPointer.state == 80)
            {
                std::cout << "## Caught abort key-command. Exiting loop..." << std::endl;
                running = false;
                XRecordDisableContext(display, context);
                XRecordFreeContext(display, context);
                XCloseDisplay(display);
                if(writer)
                {
                    fileWriter.close();
                    delete writer;
                }
                exit(0);// Why does the loop not exit?
            }
        break;

        case ButtonPress:
        if(datum->event.u.u.detail == Button1)
        {
            saveCurrentCamera(datum->event.u.keyButtonPointer.rootX,
                              datum->event.u.keyButtonPointer.rootY,
                              true);
        }

        break;

        case MotionNotify:
            if(timeUp())
                saveCurrentCamera(datum->event.u.keyButtonPointer.rootX,
                                  datum->event.u.keyButtonPointer.rootY,
                                  false);
        break;

    }

    XRecordFreeData(data);
}

int main(int argc, char *argv[])
{
    int camID;

    camID = 0;

    std::cout << "## CalibrationDaemon has been started." << std::endl;

    if(argc == 2)
        filepath = argv[1];

    std::cout << "## Setting up target directory \"" << filepath << "\"..." << std::endl;
    std::string call("mkdir -p ");
    call += filepath;
    call += "";
    system(call.c_str());

    std::cout << "## Trying to open camera device (" << camID << ")" << std::endl;
    cam = cv::VideoCapture(camID);    
    if(!cam.isOpened())
    {
        std::cerr << "Failed to open camera (" << camID << "). Terminating." << std::endl;
        exit(0);
    }

    cv::Size frameSize(1280, 800);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, frameSize.width);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, frameSize.height);

    std::cout << "## Waiting for camera...";

    {
        cv::Mat temp;
        cam >> temp;
        while(temp.size().area() == 0)
        {
            cam >> temp;
            std::cout << ".";
        }
        std::cout << std::endl;
    }

    std::cout << "## Setting up listener..." << std::endl;

    display = XOpenDisplay(0);
    if(!display)
    {
        std::cerr << "Unable to open default display. Remote displays are not supported!" << std::endl;
        exit(1);
    }

    XRecordClientSpec clients = XRecordAllClients;
    XRecordRange * range = XRecordAllocRange();
    if(!range)
    {
        std::cerr << "Error allocating XRecordRange. Terminating" << std::endl;
        exit(1);
    }

    range->device_events.first = KeyPress;
    range->device_events.last = MotionNotify;

    context = XRecordCreateContext(display, 0, &clients, 1, &range, 1);
    XFree(range);
    if(!context)
    {
        std::cerr << "Error creating XRecordContext. Terminating" << std::endl;
        exit(1);
    }

    std::cout << "## Waiting for mouse click events... (Press Meta+F12 to exit this program)" << std::endl;
    XRecordEnableContext(display, context, eventCallback, NULL);
    while(running)
    {
        XRecordProcessReplies(display);
    }

    std::cout << "## Cleaning up... ";
    //cam.release();
    XRecordDisableContext(display, context);
    XRecordFreeContext(display, context);
    XCloseDisplay(display);
    std::cout << "## Done" << std::endl;
}
