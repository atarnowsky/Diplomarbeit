#include "wega.h"
#include "interfaces.h"

#include "tracking/fulltemplatelocator.h"
#include "tracking/histthreshold.h"
#include "tracking/defaultpupilselector.h"
#include "tracking/defaultglintselector.h"
#include "tracking/trivialmidpointextractor.h"
#include "tracking/momentummidpointextractor.h"
#include "tracking/leastsquaresellipsefitter.h"
#include "tracking/optimizedleastsquares.h"
#include "tracking/enhancedleastsquares.h"
#include "invariantTracking/defaultcornerextractor.h"
#include "invariantTracking/cornersorter.h"
#include "randomdomainplacer.h"
#include "kmeansplacer.h"
#include "rolfplacer.h"
#include "delaunay/linearinterpolator.h"

#include <cfloat>

namespace WeGA {

FixedHeadTracker::FixedHeadTracker(
        FixedHeadTracker::ExtractionMethod pupilMethod,
        FixedHeadTracker::ExtractionMethod glintMethod,
        TrackingTunables &tunables,
        DebugInfo *debug) : tunables(tunables)
{
    scaleFactor = 1.0;
    MidpointExtractor* extractorPupil = 0;
    switch(pupilMethod)
    {
    case BoundingBox:
        extractorPupil = new TrivialMidpointExtractor();
    break;

    case Centroid:
        extractorPupil = new MomentumMidpointExtractor(true);
    break;

    case LeastSquares:
        extractorPupil = new LeastSquaresEllipseFitter();
    break;

    case BinaryLeastSquares:
        extractorPupil = new BinaryLeastSquaresEllipseFitter(currentGlint, scaleFactor);
    break;

    case ComplexLeastSquares:
        extractorPupil = new ComplexLeastSquaresEllipseFitter(scaleFactor, threshLow);
    break;
    }

    MidpointExtractor* extractorGlint = 0;
    switch(glintMethod)
    {
    case BoundingBox:
        extractorGlint = new TrivialMidpointExtractor();
    break;

    case Centroid:
        extractorGlint = new MomentumMidpointExtractor(false);
    break;

    case LeastSquares:
        extractorGlint = new LeastSquaresEllipseFitter();
    break;

    case BinaryLeastSquares: // Fallback
        extractorGlint = new LeastSquaresEllipseFitter();
    break;

    case ComplexLeastSquares:
        extractorGlint = new ComplexLeastSquaresEllipseFitter(scaleFactor, threshHigh);
    break;
    }

    FixedHeadTracker(0, 0, 0, 0, 0, extractorPupil, extractorGlint, tunables, debug);
}

FixedHeadTracker::FixedHeadTracker(
        EyeLocator*        eyeLocator,
        ThresholdFilter*   thresholdPupil,
        ThresholdFilter*   thresholdGlint,
        RegionSelector*    regionPupil,
        RegionSelector*    regionGlint,
        MidpointExtractor* fitterPupil,
        MidpointExtractor* fitterGlint,
        TrackingTunables& tunables,
        DebugInfo* debug) :
    debug(debug), scaleFactor(1.0), tunables(tunables)
{
    reset();

    locateEye = (eyeLocator)? eyeLocator :
                              new FullTemplateLocator(tunables.eyeTemplate);

    pupilThreshold = (thresholdPupil)? thresholdPupil:
                                       new HistThreshold(tunables.pupilSize);

    glintThreshold = (thresholdGlint)? thresholdGlint:
                                       new HistThreshold(tunables.glintSize);

    getPupil = (regionPupil)? regionPupil :
                              new DefaultPupilSelector(scaleFactor);

    getGlint = (regionGlint)? regionGlint :
                              new DefaultGlintSelector(scaleFactor, currentPupil);

    extractPupil = (fitterPupil)? fitterPupil :
                                  new BinaryLeastSquaresEllipseFitter(currentGlint, scaleFactor);

    extractGlint = (fitterGlint)? fitterGlint :
                                  new MomentumMidpointExtractor(false);
}

TrackingResult FixedHeadTracker::analyzeFrame(const cv::Mat& image)
{

    vector2 corners[4] = {vector2(0,0), vector2(0,0), vector2(0,0), vector2(0,0)};
    TrackingResult::CornerStatus status[4] = {
        TrackingResult::Missing, TrackingResult::Missing,
        TrackingResult::Missing, TrackingResult::Missing
    };

    // (0) Acquire image and convert to grayscale
    cv::Size frameSize = image.size();
    scaleFactor = frameSize.width/640.0;
    grayImage = cv::Mat(image);

    // (1) Search for eye region and crop image
    eyeRegion = locateEye->run(grayImage);
    eyeImage = cv::Mat(grayImage, eyeRegion);
    cv::transpose(eyeImage, eyeImage);
    cv::flip(eyeImage, eyeImage, 0);
    if(debug)
    {
        debug->eyeRegion = eyeRegion;
        std::swap(debug->eyeRegion.width, debug->eyeRegion.height);
        std::swap(debug->eyeRegion.x, debug->eyeRegion.y);
        debug->eyeRegion.y = frameSize.height - debug->eyeRegion.y;
    }

    if(eyeRegion.area() == 0.0)
        return TrackingResult();

    // (2) Hi- and Low- Thresholding
    cv::Mat binaryPupil, binaryGlint;
    threshLow  = pupilThreshold->run(eyeImage, binaryPupil);
    threshHigh = glintThreshold->run(eyeImage, binaryGlint);

    // (3a) Pupil region selection
    cv::Rect pupilRegion; cv::Mat pupilPixels;
    pupilRegion = getPupil->run(binaryPupil, eyeImage, pupilPixels);

    // (3b) Glint region selection
    cv::Rect glintRegion; cv::Mat glintPixels;
    glintRegion = getGlint->run(binaryGlint, eyeImage, glintPixels);

    // (4a) Glint region midpoint extraction
    cv::Mat glintGray(eyeImage, glintRegion);
    glintEllipse = extractGlint->run(glintPixels, glintGray);
    glintEllipse.center.x += glintRegion.x;
    glintEllipse.center.y += glintRegion.y;
    currentGlint = glintEllipse.center;

    // (4b) Pupil region midpoint extraction
    cv::Mat pupilGray(eyeImage, pupilRegion);
    currentGlint.x -= pupilRegion.x;
    currentGlint.y -= pupilRegion.y;
    pupilEllipse = extractPupil->run(pupilPixels, pupilGray);
    pupilEllipse.center.x += pupilRegion.x;
    pupilEllipse.center.y += pupilRegion.y;
    currentPupil = pupilEllipse.center;

    // (5) Result vector calculation
    vector2 PGV;
    vector2 absolutePupil, absoluteGlint;

    if(pupilPixels.size().area() > 0)
        lastPupil = pupilEllipse.center;

    if(glintPixels.size().area() > 0)
        lastGlint = glintEllipse.center;

    vector2 temp      = vector2(eyeRegion.y, frameSize.width -
                               (eyeRegion.x + eyeRegion.width));
    absolutePupil     = vector2(lastPupil.x, lastPupil.y) + temp;
    absolutePupil.x  /= scaleFactor;
    absolutePupil.y  /= scaleFactor;
    absoluteGlint     = vector2(lastGlint.x, lastGlint.y) + temp;
    absoluteGlint.x  /= scaleFactor;
    absoluteGlint.y  /= scaleFactor;
    PGV               = lastPupil - lastGlint;
    PGV.x            /= scaleFactor;
    PGV.y            /= scaleFactor;

    if(debug)
    {
        pupilEllipse.center.x += temp.x;
        pupilEllipse.center.y += temp.y;
        glintEllipse.center.x += temp.x;
        glintEllipse.center.y += temp.y;

        debug->glintEllipse = glintEllipse;
        debug->pupilEllipse = pupilEllipse;
    }

    return TrackingResult(absolutePupil, absoluteGlint, corners, status);
}

void FixedHeadTracker::reset()
{
    lastPupil = vector2(-1, -1);
    lastGlint = vector2(-1, -1);
    currentPupil = vector2(-1, -1);
    currentGlint = vector2(-1, -1);
}

InvariantHeadTracker::InvariantHeadTracker(
        EyeLocator*        eyeLocator,
        ThresholdFilter*   thresholdPupil,
        ThresholdFilter*   thresholdGlint,
        RegionSelector*    regionPupil,
        RegionSelector*    regionGlint,
        MidpointExtractor* fitterPupil,
        MidpointExtractor* fitterGlint,
        TrackingTunables &tunables,
        DebugInfo* debug) :
    FixedHeadTracker(eyeLocator, thresholdPupil, thresholdGlint,
                     regionPupil, regionGlint, fitterPupil, fitterGlint, tunables, debug)
{
    lastArea = 0.0;
    lastRating = 0.0;
}

InvariantHeadTracker::InvariantHeadTracker(
        FixedHeadTracker::ExtractionMethod pupilMethod,
        FixedHeadTracker::ExtractionMethod glintMethod,
        TrackingTunables &tunables,
        DebugInfo *debug) :
    FixedHeadTracker(pupilMethod, glintMethod, tunables, debug)
{
    lastArea = 0.0;
    lastRating = 0.0;
}

inline scalar distance(const vector2& a, const vector2& b)
{
    vector2 d = a - b;
    return sqrt(d.x*d.x+d.y*d.y);
}

inline vector2 normalized(const vector2& a)
{
    scalar l = sqrt(a.x*a.x+a.y*a.y);
    return vector2(a.x/l, a.y/l);
}

inline scalar rateSet(const std::vector<vector2>& candidates)
{
    scalar result = 0.0;
    for(unsigned int index = 0; index < candidates.size(); index++)
    {
        unsigned int neighbor = index;
        scalar neigVal = FLT_MAX;
        for(unsigned int n = 0; n < candidates.size(); n++)
        {
            if(n == index) continue;
            scalar dist = distance(candidates[index], candidates[n]);
            if(dist < neigVal)
            {
                neigVal = dist;
                neighbor = n;
            }
        }

        vector2 diff = normalized(candidates[index] - candidates[neighbor]);
        for(unsigned int n = 0; n < candidates.size(); n++)
        {
            if(n == index || n == neighbor) continue;
            result += fabs(diff.dot(normalized(candidates[index] - candidates[n])));
        }
    }

    return result;
}

inline scalar triangleArea(const vector2& a, const vector2& b, const vector2& c)
{
    return fabs(0.5 * (a.x*(b.y-c.y)+b.x*(c.y-a.y)+c.x*(a.y-b.y)));
}

std::vector<vector2> InvariantHeadTracker::bestCandidates(const std::vector<vector2>& candidates,
                                                          const cv::Mat& cornerGray,
                                                          const cv::Rect& searchRegion) const
{
    std::vector<vector2> result;


    for(unsigned int n = 0; n < candidates.size(); n++)
    {
        vector2 g(candidates[n]);
        int boundSize = 8*scaleFactor;
        bool skip = false;
        for(unsigned int m = 0; m < candidates.size(); m++)
        {
            if(m == n) continue;
            scalar d = distance(g, candidates[m]);
            if(d < boundSize*2)
            {
                skip = true;
                break;
            }
        }

        cv::Rect bound(g.x - boundSize, g.y - boundSize, 2*boundSize, 2*boundSize);
        if(bound.tl().x < 0 || bound.tl().y < 0 || bound.br().x > cornerGray.cols || bound.br().y > cornerGray.rows || skip)
            continue;

        result.push_back(g);
    }

    return result;
}

TrackingResult InvariantHeadTracker::analyzeFrame(const cv::Mat& image)
{
    const scalar histFactor = 0.75;
    TrackingResult oldResult = FixedHeadTracker::analyzeFrame(image);
    vector2 oldPupil = oldResult.pupilPosition();

    int regionSize = 50 * scaleFactor;
    const scalar aspectRatio = 16.0/10.0;
    const scalar offsetX = regionSize * 0.25;
    cv::Rect searchRegion(lastGlint.x - regionSize + offsetX, lastGlint.y - regionSize/aspectRatio, 2*regionSize, 2*regionSize/aspectRatio);
    if(searchRegion.tl().x < 0 || searchRegion.tl().y < 0 || searchRegion.br().x >= eyeImage.cols || searchRegion.br().y >= eyeImage.rows)
        return oldResult;
    cv::Mat cornerThresh;
    cv::Mat cornerGray(eyeImage, searchRegion);

    scalar threshVal = 0.015;
    std::vector<vector2> candidates;

    while(candidates.size() < 3 && threshVal < 0.03)
    {
        HistThreshold threshold(-threshVal, histFactor);
        threshold.run(cornerGray, cornerThresh);

        DefaultCornerExtractor extractCorners(this->scaleFactor);
        candidates = bestCandidates(extractCorners.run(cornerThresh, cornerGray), cornerGray, searchRegion);
        threshVal += 0.00125;
    }

    if(candidates.size() < 3)
        return oldResult;

    vector2 glint(lastGlint.x - searchRegion.tl().x, lastGlint.y - searchRegion.tl().y);

    scalar bestRating = FLT_MAX;
    scalar bestSingle = 0.0;
    std::vector<vector2> bestSet;
    for(unsigned int a = 0; a < candidates.size() - 2; a++)
        for(unsigned int b = a+1; b < candidates.size() - 1; b++)
            for(unsigned int c = b+1; c < candidates.size(); c++)
            {
                std::vector<vector2> tempSet;
                tempSet.push_back(candidates[a]);
                tempSet.push_back(candidates[b]);
                tempSet.push_back(candidates[c]);

                scalar currentRating = rateSet(tempSet) * 5000 / scaleFactor;

                vector2 midPoint = (tempSet[0] + tempSet[1] + tempSet[2]);
                midPoint.x /= 3.0;
                midPoint.y /= 3.0;

                scalar glintDistance = distance(midPoint, glint);
                glintDistance *= glintDistance;
                glintDistance *= 10;

                scalar areaDifference = 0;

                if(lastArea > 0.0)
                {
                    areaDifference = fabs(
                            triangleArea(tempSet[0],tempSet[1],tempSet[2]) -
                            lastArea) * 5;
                }

                scalar ratingDifference = 0;

                if(lastRating > 0.0)
                {
                    ratingDifference = fabs(lastRating - currentRating);
                }

                scalar combinedRating = currentRating + glintDistance + areaDifference + ratingDifference;

                if(combinedRating < bestRating)
                {
                    bestSingle = currentRating;
                    bestRating = combinedRating;
                    bestSet = tempSet;
                }
            }

    if(bestRating > 15000)
        return oldResult;

    if(lastArea == 0.0)
        lastArea = triangleArea(bestSet[0],bestSet[1],bestSet[2]);
    else
        lastArea = lastArea * 0.8 + triangleArea(bestSet[0],bestSet[1],bestSet[2]) * 0.2;

    if(lastRating == 0.0)
        lastRating = bestSingle;
    else
        lastRating = lastRating * 0.6 + bestSingle * 0.4;

    std::vector<TrackingResult::CornerStatus> statuslist;
    statuslist.push_back(TrackingResult::Found);
    statuslist.push_back(TrackingResult::Found);
    statuslist.push_back(TrackingResult::Found);

    CornerSorter sorter;
    sorter.run(bestSet, statuslist);

    const scalar maxDist = 8*scaleFactor;
    for(int n = 0; n < 4; n++)
    {
        if(statuslist[n] == TrackingResult::Interpolated)
        {
            vector2 elem = bestSet[n];
            bool found = false;
            for(unsigned int m = 0; m < candidates.size(); m++)
            {
                vector2 target = candidates[m];
                scalar dist = distance(elem, target);
                if(dist < maxDist)
                {
                    elem = target;
                    found = true;
                    break;
                }
            }
            if(found)
            {
                bestSet[n] = elem;
                statuslist[n] = TrackingResult::Found;
            }
        }
    }

    vector2 offset = vector2(searchRegion.tl().x, searchRegion.tl().y) +
            vector2(eyeRegion.y, scaleFactor * 640 - (eyeRegion.x + eyeRegion.width));
    vector2 corners[4] = {bestSet[0]*(1.0/scaleFactor) + offset*(1.0/scaleFactor),
                          bestSet[1]*(1.0/scaleFactor) + offset*(1.0/scaleFactor),
                          bestSet[2]*(1.0/scaleFactor) + offset*(1.0/scaleFactor),
                          bestSet[3]*(1.0/scaleFactor) + offset*(1.0/scaleFactor)};

    TrackingResult::CornerStatus status[4] = {
        statuslist[0], statuslist[1], statuslist[2], statuslist[3]
    };

    return TrackingResult(oldPupil, oldResult.glintPosition(),
                          corners, status);
}

void InvariantHeadTracker::reset()
{
    FixedHeadTracker::reset();
    lastArea = 0.0;
    lastRating = 0.0;
}

FixedHeadMapper::FixedHeadMapper(Approximator<2,2>* interpolator) :
    type(Gaussian),
    interpolator(interpolator)
{

}

FixedHeadMapper::FixedHeadMapper(InterpolationType type) :
    type(type),
    interpolator(0)
{
    switch(type)
    {
    case Linear:
        interpolator = new LinearInterpolator();
        break;

    case Gaussian:
        interpolator = new InterpolationTrainer<GaussianFunction, EuclideanMetric, 2, 2>(0.01);
        break;

    case Multiquadric:
        interpolator = new InterpolationTrainer<MultiquadricFunction, EuclideanMetric, 2, 2>(0.01);
        break;

    case SphericFull:
    case SphericAuto:
        interpolator = new InterpolationTrainer<BumpFunction, EuclideanMetric, 2, 2>(0.01);
        break;
    }
}

MappingResult FixedHeadMapper::mapToScreen(const TrackingResult& inputVector)
{
    vector<2> input;
    input[0] = inputVector.pupilGlintVector().x;
    input[1] = inputVector.pupilGlintVector().y;

    return MappingResult(vector2(interpolator->evaluate(input)));
}

void FixedHeadMapper::addTrainingData(
        const std::vector< std::pair<TrackingResult, MappingResult> >& data)
{
    for(unsigned int n = 0; n < data.size(); n++)
    {
        trainingInput.push_back(data[n]);

        vector<2> in; vector<2> out;

        in[0] = data[n].first.pupilGlintVector().x;
        in[1] = data[n].first.pupilGlintVector().y;

        out[0] = data[n].second.position().x;
        out[1] = data[n].second.position().y;

        if(type == SphericAuto)
        {
            if(
                    (out[0] == 0.5 && out[1] <= 0.25) ||
                    (out[0] == 0.5 && out[1] >= 0.75) ||
                    (out[0] <= 0.25 && out[1] == 0.5) ||
                    (out[0] >= 0.75 && out[1] == 0.5)
              )
            {
                sphericTests.push_back(std::make_pair(in,out));
                continue;
            }
        }

        interpolator->addTrainingTuple(in, out);
    }
    if(type == SphericAuto)
        sphericOptimization();
    else
        interpolator->update();
}

scalar FixedHeadMapper::trainingDataDiameter()
{
    scalar largest = 0.0;
    for(unsigned int n = 0; n < trainingInput.size(); n++)
        for(unsigned int m = n; m < trainingInput.size(); m++)
        {
            scalar x = trainingInput[n].first.pupilGlintVector().x;
            scalar y = trainingInput[n].first.pupilGlintVector().y;
            scalar x2 = trainingInput[m].first.pupilGlintVector().x;
            scalar y2 = trainingInput[m].first.pupilGlintVector().y;
            scalar distance = (x-x2)*(x-x2)+(y-y2)*(y-y2);
            if(distance > largest)
                largest = distance;
        }
    return sqrt(largest);
}

void FixedHeadMapper::setKernelWidth(scalar width)
{
    interpolator->update(width);
}

scalar FixedHeadMapper::sphericOptimization()
{
    if(type != SphericAuto)
        return -1;

    scalar currentWidth = 1.0;
    scalar lastErr = 999.0;
    scalar bestErr = 999.0;
    scalar bestWidth = 1.0;
    bool bestFound = false;
    scalar diam = trainingDataDiameter();
    while(!bestFound && currentWidth > 0.0)
    {
        this->setKernelWidth(currentWidth / diam);

        scalar totalErr = 0.0;
        for(unsigned int n = 0; n < sphericTests.size(); n++)
        {
            vector2 err = vector2(interpolator->evaluate(vector<2>(sphericTests[n].first))) - sphericTests[n].second;
            totalErr += fabs(err.x)+fabs(err.y);
        }

        if(totalErr < bestErr)
        {
            bestErr = totalErr;
            bestWidth = currentWidth;
        }
        else if(totalErr > lastErr)
        {
            bestFound = true;
            break;
        }
        currentWidth -= 0.001;
    }

    this->setKernelWidth(bestWidth / diam);
    return bestWidth;
}


InvariantHeadMapper::InvariantHeadMapper(
        NeuronPlacer<8>* placer,
        Approximator<8, 2>* approximator)
{
    this->placer = placer;
    this->approximator = approximator;
}

InvariantHeadMapper::InvariantHeadMapper(PlacementStrategy strategy,
                                         unsigned int neuronCount,
                                         KernelType type,
                                         scalar rolfR, scalar rolfLR,
                                         scalar rolfLP, scalar rolfIR) :
    neurons(neuronCount)
{
    switch(strategy)
    {
    case Random:
        placer = new RandomDomainPlacer<8>(true,neuronCount);
        break;
    case KMeans:
        placer = new KMeansPlacer<8>(true,neuronCount);
        break;
    case ROLF:
        placer = new ROLFPlacer<8>(rolfR,     // Radius multiplicator
                                   rolfLR,   // Learningrate radius
                                   rolfLP,   // Learningrate position
                                   rolfIR,   // Initial radius for first neuron
                                   neuronCount);
        break;
    }

    switch(type)
    {
    case Gaussian:
        approximator =
                new ApproximationTrainer<GaussianFunction, EuclideanMetric, 8, 2>(*placer, 0.01);
        break;
    case Spheric:
        approximator =
                new ApproximationTrainer<BumpFunction, EuclideanMetric, 8, 2>(*placer, 0.01);
        break;
    case Multiquadric:
        approximator =
                new ApproximationTrainer<MultiquadricFunction, EuclideanMetric, 8, 2>(*placer, 0.01);
        break;
    }
}

InvariantHeadMapper::~InvariantHeadMapper()
{
    delete placer;
    delete approximator;
}

MappingResult InvariantHeadMapper::mapToScreen(const TrackingResult& inputVector)
{
    vector<8> v;
    v[0] = inputVector.pupilGlintVector().x;
    v[1] = inputVector.pupilGlintVector().y;
    v[2] = inputVector.cornerVector(TrackingResult::TopLeft).x;
    v[3] = inputVector.cornerVector(TrackingResult::TopLeft).y;
    v[4] = inputVector.cornerVector(TrackingResult::TopRight).x;
    v[5] = inputVector.cornerVector(TrackingResult::TopRight).y;
    v[6] = inputVector.cornerVector(TrackingResult::BottomLeft).x;
    v[7] = inputVector.cornerVector(TrackingResult::BottomLeft).y;

    return MappingResult(vector2(approximator->evaluate(v)));
}

void InvariantHeadMapper::addTrainingData(const std::vector<TrainingPair>& data)
{
    for(unsigned int n = 0; n < data.size(); n++)
    {
        vector<8> v;
        v[0] = data[n].first.pupilGlintVector().x;
        v[1] = data[n].first.pupilGlintVector().y;
        v[2] = data[n].first.cornerVector(TrackingResult::TopLeft).x;
        v[3] = data[n].first.cornerVector(TrackingResult::TopLeft).y;
        v[4] = data[n].first.cornerVector(TrackingResult::TopRight).x;
        v[5] = data[n].first.cornerVector(TrackingResult::TopRight).y;
        v[6] = data[n].first.cornerVector(TrackingResult::BottomLeft).x;
        v[7] = data[n].first.cornerVector(TrackingResult::BottomLeft).y;
        placer->addVector(v);

        vector<2> out;
        out[0] = data[n].second.position().x;
        out[1] = data[n].second.position().y;

        approximator->addTrainingTuple(v, out);
    }

    scalar diam = placer->positionsDiameter();
    approximator->update(neurons/(diam*diam));
}


CrossRatioMapper::CrossRatioMapper()
{
    alpha[0] = 2.0;
    alpha[1] = 2.0;
    alpha[2] = 2.0;
    alpha[3] = 2.0;
}

vector2 crossing(vector2 p1, vector2 p2, vector2 q1, vector2 q2)
{
    vector2 p2o(-p2.y, p2.x);
    vector2 q2o(-q2.y, q2.x);
    return (q1.ddot(q2o)*p2 - p1.ddot(p2o)*q2)*(1.0/p2.ddot(q2o));
}

vector2 interpolateGlint(const TrackingResult& in)
{
    if(in.corner(TrackingResult::BottomRight).second == TrackingResult::Found)
        return in.corner(TrackingResult::BottomRight).first;

    vector2 a = in.corner(TrackingResult::TopLeft).first;
    vector2 b = in.corner(TrackingResult::TopRight).first;
    vector2 c = in.corner(TrackingResult::BottomLeft).first;

    vector2 m = (a+b+c)*(1.0/3.0);
    vector2 d = m + (m-a)*1.025;
    return d;
}

void CrossRatioMapper::premapCorners(const TrackingResult& inputVector,
                                     vector2 array[])
{
    scalar offs = 0.0;
    vector2 cMid = vector2(320,240)* offs + inputVector.glintPosition() * (1.0-offs);

    vector2 gc[4];
    gc[0] = inputVector.corner(TrackingResult::TopLeft).first - cMid;
    gc[1] = inputVector.corner(TrackingResult::TopRight).first - cMid;
    gc[2] = inputVector.corner(TrackingResult::BottomLeft).first - cMid;
    gc[3] = interpolateGlint(inputVector) - cMid;

    for(int n = 0; n < 4; n++)
        array[n] = gc[n]*alpha[n] + cMid;
}

MappingResult CrossRatioMapper::mapToScreen(const TrackingResult& inputVector)
{
    vector2 tr[4];
    premapCorners(inputVector, tr);

    vector2 vanishVert = crossing(
                tr[0], tr[2]-tr[0],
                tr[1], tr[3]-tr[1]
                );

    vector2 vanishHor = crossing(
                tr[0], tr[1]-tr[0],
                tr[2], tr[3]-tr[2]
                );

    vector2 e = crossing(
                tr[0], tr[3]-tr[0],
                tr[1], tr[2]-tr[1]
                );

    vector2 um1 = crossing(
                vanishVert, vanishVert - e,
                tr[0], tr[1]-tr[0]
                );

    vector2 um2 = crossing(
                vanishVert, vanishVert - inputVector.pupilPosition(),
                tr[0], tr[1]-tr[0]
                );

    vector2 um3 = crossing(
                vanishHor, vanishHor - inputVector.pupilPosition(),
                tr[0], tr[2]-tr[0]
                );

    vector2 um4 = crossing(
                vanishHor, vanishHor - e,
                tr[0], tr[2]-tr[0]
                );

    scalar CRx = ((tr[1].x*um1.y - um1.x*tr[1].y) * (um2.x*tr[0].y - tr[0].x*um2.y))
                /((tr[1].x*um2.y - um2.x*tr[1].y) * (um1.x*tr[0].y - tr[0].x*um1.y));

    scalar CRy = ((tr[0].x*um3.y - um3.x*tr[0].y) * (um4.x*tr[2].y - tr[2].x*um4.y))
                /((tr[0].x*um4.y - um4.x*tr[0].y) * (um3.x*tr[2].y - tr[2].x*um3.y));


    return MappingResult(vector2(1.0 - CRx/(1+CRx), CRy/(1+CRy)));
}

inline scalar dist(vector2 p, vector2 r)
{
    scalar dx = p.x - r.x;
    scalar dy = p.y - r.y;
    return sqrt(dx*dx+dy*dy);
}

void CrossRatioMapper::addTrainingData(
        const std::vector<TrainingPair>& data)
{
    scalar newAlpha[4] = {0.0};
    int alphaCount[4] = {0};

    for(unsigned int n = 0; n < data.size(); n++)
    {
        TrackingResult tr = data[n].first;
        MappingResult mp = data[n].second;

        if(mp.position().x == 0.0 && mp.position().y == 0.0)
        {
            newAlpha[0] += dist(tr.pupilPosition(), tr.glintPosition())
                    /dist(tr.corner(TrackingResult::TopLeft).first, tr.glintPosition());
            alphaCount[0]++;
        }

        if(mp.position().x == 1.0 && mp.position().y == 0.0)
        {
            newAlpha[1] += 2.0 - dist(tr.pupilPosition(), tr.glintPosition())
                    /dist(tr.corner(TrackingResult::TopRight).first, tr.glintPosition());
            alphaCount[1]++;
        }

        if(mp.position().x == 0.0 && mp.position().y == 1.0)
        {
            newAlpha[2] += dist(tr.pupilPosition(), tr.glintPosition())
                    /dist(tr.corner(TrackingResult::BottomLeft).first, tr.glintPosition());
            alphaCount[2]++;
        }

        if(mp.position().x == 1.0 && mp.position().y == 1.0)
        {
            newAlpha[3] += 2.0 - dist(tr.pupilPosition(), tr.glintPosition())
                    /dist(interpolateGlint(tr), tr.glintPosition());
            alphaCount[3]++;
        }
    }

    for(int n = 0; n < 4; n++)
    {
        if(alphaCount[n] > 0)
            alpha[n] = newAlpha[n]/scalar(alphaCount[n]);
    }
}

}
