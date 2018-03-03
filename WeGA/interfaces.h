/*******************************************************************************

    interfaces.h - Last modified 12/30/2011
    Contains interfaces for customizing WeGA through the Low-Level-API

    Part of the diploma-thesis "Head Position Invariant Gaze Tracking"
    Andreas Tarnowsky - andreas.tarnowsky@atsd.net/atarnows@welfenlab.de

    Part of the WeGA tracking library

*******************************************************************************/


#ifndef INTERFACES_H
#define INTERFACES_H

#include "gazeapi.h"

template<int length>
class vector : public cv::Vec<scalar, length>
{
public:
    vector(const cv::Vec<scalar, length>& c) :
        cv::Vec<scalar, length>(c) {}

    vector() :
        cv::Vec<scalar, length>() {}
};

namespace WeGA {

class EyeLocator
{
public:
    virtual
    cv::Rect run(const cv::Mat& input) = 0;
};

class ThresholdFilter
{
public:
    virtual
    scalar run(const cv::Mat& input,
               cv::Mat& output) = 0;
};

class RegionSelector
{
public:
    virtual
    cv::Rect run(const cv::Mat& input,
                 const cv::Mat& gray,
                 cv::Mat& region) = 0;
};

class MidpointExtractor
{
public:
    virtual
    cv::RotatedRect run(const cv::Mat& inputBinary,
                        const cv::Mat& inputGray) = 0;
};

class CornerExtractor
{
public:
    virtual
    std::vector<vector2> run(const cv::Mat& inputBinary,
                             const cv::Mat& inputGray) = 0;
};

class CornerCompleter
{
public:
    virtual
    void run(std::vector<vector2>& corners,
             std::vector<TrackingResult::CornerStatus>& status) = 0;
};


template<int in, int out>
class Approximator
{
public:
    virtual void clear() = 0;
    virtual void update() = 0;
    virtual void update(scalar) = 0;
    virtual void addTrainingTuple(const vector<in>&,
                                  const vector<out>&) = 0;
    virtual vector<out> evaluate(const vector<in>&) const = 0;
};

}

#endif // INTERFACES_H
