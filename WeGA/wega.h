/*******************************************************************************

    wega.h - Last modified 12/30/2011
    Contains the WeGA High-Level-API and algorithm implementations

    Part of the diploma-thesis "Head Position Invariant Gaze Tracking"
    Andreas Tarnowsky - andreas.tarnowsky@atsd.net/atarnows@welfenlab.de

    Part of the WeGA tracking library

*******************************************************************************/

#ifndef WEGA_H
#define WEGA_H


#include "gazeapi.h"
#include "rbfapi.h"
#include "interpolationtrainer.h"
#include "approximationtrainer.h"
#include "defaultbasisfunctions.h"
#include "defaultmetrics.h"

namespace WeGA {

class LinearInterpolator;
class FullTemplateLocator;
class InlineFilter;
class ThresholdFilter;
class RegionSelector;
class MidpointExtractor;

class FixedHeadTracker : public TrackingAlgorithm
{
public:
    struct TrackingTunables
    {
        scalar pupilSize, glintSize;
        cv::Mat eyeTemplate;
    };

    struct DebugInfo
    {
        cv::RotatedRect pupilEllipse;
        cv::RotatedRect glintEllipse;
        cv::Rect        eyeRegion;
        vector2         absolutePupil;
        vector2         absoluteGlint;
    };

    enum ExtractionMethod
    {
        BoundingBox           = 0,
        Centroid              = 1,
        LeastSquares          = 2,
        BinaryLeastSquares    = 3,
        ComplexLeastSquares   = 4
    };

    FixedHeadTracker(ExtractionMethod pupilMethod,
                     ExtractionMethod glintMethod,

                     TrackingTunables &tunables,
                     DebugInfo* debug = 0);

    FixedHeadTracker(EyeLocator*        eyeLocator,
                     ThresholdFilter*   thresholdPupil,
                     ThresholdFilter*   thresholdGlint,
                     RegionSelector*    regionPupil,
                     RegionSelector*    regionGlint,
                     MidpointExtractor* fitterPupil,
                     MidpointExtractor* fitterGlint,

                     TrackingTunables& tunables,
                     DebugInfo* debug = 0);

    virtual TrackingResult analyzeFrame(const cv::Mat& image);
    virtual void reset();

protected:
    vector2 lastPupil, lastGlint,
            currentPupil, currentGlint;

    DebugInfo* debug;
    scalar scaleFactor, threshLow, threshHigh;

    cv::Mat grayImage, eyeImage;
    cv::Rect eyeRegion;
    cv::RotatedRect pupilEllipse, glintEllipse;

private:
    EyeLocator*            locateEye;
    ThresholdFilter*       pupilThreshold;
    ThresholdFilter*       glintThreshold;
    RegionSelector*        getPupil;
    RegionSelector*        getGlint;
    MidpointExtractor*     extractPupil;
    MidpointExtractor*     extractGlint;

    TrackingTunables&      tunables;
};

class InvariantHeadTracker : public FixedHeadTracker
{
public:
    InvariantHeadTracker(ExtractionMethod pupilMethod,
                     ExtractionMethod glintMethod,

                     TrackingTunables &tunables,
                     DebugInfo* debug = 0);

    InvariantHeadTracker(EyeLocator*        eyeLocator,
                         ThresholdFilter*   thresholdPupil,
                         ThresholdFilter*   thresholdGlint,
                         RegionSelector*    regionPupil,
                         RegionSelector*    regionGlint,
                         MidpointExtractor* fitterPupil,
                         MidpointExtractor* fitterGlint,

                         TrackingTunables& tunables,
                         DebugInfo* debug = 0);

    virtual TrackingResult analyzeFrame(const cv::Mat& image);
    virtual void reset();

protected:
    std::vector<vector2> bestCandidates(const std::vector<vector2>& candidates,
                                        const cv::Mat& cornerGray,
                                        const cv::Rect& searchRegion) const;

    scalar lastArea;
    scalar lastRating;
};


class FixedHeadMapper : public MappingAlgorithm
{
public:
    enum InterpolationType
    {
        Linear,
        Gaussian,
        Multiquadric,
        SphericFull,
        SphericAuto
    };

    typedef std::pair<TrackingResult, MappingResult> TrainingPair;

    FixedHeadMapper(Approximator<2,2>* interpolator);
    FixedHeadMapper(InterpolationType type);

    virtual MappingResult mapToScreen(const TrackingResult& inputVector);
    virtual void addTrainingData(
            const std::vector<TrainingPair>& data);

    scalar trainingDataDiameter();
    void setKernelWidth(scalar width);

protected:
    scalar sphericOptimization();

private:
    InterpolationType type;
    std::vector<TrainingPair> trainingInput;
    std::vector<std::pair<vector2,vector2> > sphericTests;

    Approximator<2,2>* interpolator;
};

class InvariantHeadMapper : public MappingAlgorithm
{
public:
    enum KernelType
    {
        Gaussian,
        Spheric,
        Multiquadric
    };

    enum PlacementStrategy
    {
        Random,
        KMeans,
        ROLF
    };

    typedef std::pair<TrackingResult, MappingResult> TrainingPair;

    InvariantHeadMapper(NeuronPlacer<8>* placer,
                        Approximator<8, 2>* approximator);

    InvariantHeadMapper(PlacementStrategy strategy = KMeans,
                        unsigned int neuronCount = 8,
                        KernelType type = Gaussian,
                        scalar rolfR = 2.0, scalar rolfLR = 0.025,
                        scalar rolfLP = 0.025, scalar rolfIR = 10.0);

    ~InvariantHeadMapper();

    virtual MappingResult mapToScreen(const TrackingResult& inputVector);

    virtual void addTrainingData(
            const std::vector<TrainingPair>& data);

private:
    unsigned int neurons;
    NeuronPlacer<8>* placer;
    Approximator<8, 2>* approximator;
};

class CrossRatioMapper : public MappingAlgorithm
{

public:
    typedef std::pair<TrackingResult, MappingResult> TrainingPair;

    CrossRatioMapper();

    virtual MappingResult mapToScreen(const TrackingResult& inputVector);

    virtual void addTrainingData(
            const std::vector<TrainingPair>& data);

private:
    virtual void premapCorners(const TrackingResult& inputVector, vector2 array[]);
    scalar alpha[4];
};


}

#endif // WEGA_H
