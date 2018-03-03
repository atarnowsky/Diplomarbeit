/*******************************************************************************

    gazeapi.h - Last modified 12/30/2011
    Contains interfaces and container classes needed for the High-Level-API

    Part of the diploma-thesis "Head Position Invariant Gaze Tracking"
    Andreas Tarnowsky - andreas.tarnowsky@atsd.net/atarnows@welfenlab.de

    Part of the WeGA tracking library

*******************************************************************************/

#ifndef GAZEAPI_H
#define GAZEAPI_H

#include <opencv2/opencv.hpp>
#include <string>

typedef cv::Point2d vector2;
typedef cv::Point2i vector2i;
typedef double scalar;
const scalar SCALAR_EPS = std::numeric_limits<scalar>::epsilon();

namespace WeGA {

// IO classes ---

class FrameReader
{
public:
    FrameReader() {}
    virtual ~FrameReader() {}

    virtual cv::Mat nextFrame() = 0;
    virtual bool hasNext() = 0;
};

// Container classes for return values ---

class TrackingResult
{
public:
    enum CornerStatus
    {
        Found = 0,
        Interpolated = 1,
        Missing = -1
    };

    enum CornerPosition
    {
        TopLeft = 0,
        TopRight = 1,
        BottomLeft = 2,
        BottomRight = 3
    };

    TrackingResult();
    TrackingResult(const vector2& pupil,
                   const vector2& glint,
                   vector2 corners[4],
                   CornerStatus cornerStatus[4]);

    vector2 pupilPosition() const;
    vector2 glintPosition() const;
    std::pair<vector2, CornerStatus> corner(CornerPosition position) const;

    std::string toString();
    static TrackingResult fromString(const std::string& string,
                                     unsigned int offset = 0);

    // Additional helper functions not described in thesis ---------------------
    vector2 pupilGlintVector() const;
    vector2 cornerVector(CornerPosition position) const;
    // -------------------------------------------------------------------------

private:
    vector2 pupil;
    vector2 glint;
    vector2 corners[4];
    CornerStatus cornerStatus[4];
};

class MappingResult
{
public:
    MappingResult();
    MappingResult(const vector2& position);

    vector2 position() const;
    vector2i scaledPosition(vector2i screenSize) const;

    std::string toString();
    static MappingResult fromString(const std::string& string,
                                    unsigned int offset = 0);

private:
    vector2 pos;
};

// Algorithm interfaces ---

class TrackingAlgorithm
{
public:
    TrackingAlgorithm() {}

    virtual TrackingResult analyzeFrame(const cv::Mat& image) = 0;
    virtual void reset() = 0;
};

class MappingAlgorithm
{
public:
    MappingAlgorithm() {}

    virtual MappingResult mapToScreen(const TrackingResult& inputVector) = 0;
    virtual void addTrainingData(
            const std::vector<
            std::pair<TrackingResult, MappingResult> >& data) = 0;
};

}

#endif // GAZEAPI_H
