#include <iostream>
#include <vector>
#include <QFile>
#include <QString>
#include <QMap>
#include <QPoint>
#include <QStringList>
#include <cstdlib>
#include <sys/time.h>

#include "WeGA/wega.h"

using namespace WeGA;
using WeGA::InvariantHeadMapper;
typedef WeGA::InvariantHeadMapper::TrainingPair TrainingPair;
typedef WeGA::InvariantHeadMapper::KernelType KernelType;
typedef WeGA::InvariantHeadMapper::PlacementStrategy PlacementStrategy;

scalar distance(const TrackingResult& a, const TrackingResult& b)
{
    scalar d1 = a.pupilGlintVector().x - b.pupilGlintVector().x;
    scalar d2 = a.pupilGlintVector().y - b.pupilGlintVector().y;
    scalar d3 = a.cornerVector(TrackingResult::TopLeft).x - b.cornerVector(TrackingResult::TopLeft).x;
    scalar d4 = a.cornerVector(TrackingResult::TopLeft).y - b.cornerVector(TrackingResult::TopLeft).y;
    scalar d5 = a.cornerVector(TrackingResult::TopRight).x - b.cornerVector(TrackingResult::TopRight).x;
    scalar d6 = a.cornerVector(TrackingResult::TopRight).y - b.cornerVector(TrackingResult::TopRight).y;
    scalar d7 = a.cornerVector(TrackingResult::BottomLeft).x - b.cornerVector(TrackingResult::BottomLeft).x;
    scalar d8 = a.cornerVector(TrackingResult::BottomLeft).y - b.cornerVector(TrackingResult::BottomLeft).y;

    return sqrt(d1*d1+d2*d2+d3*d3+d4*d4+d5*d5+d6*d6+d7*d7+d8*d8);
}

scalar kDistance(const std::vector<TrackingResult>& dataSet, const TrackingResult& A, int k, std::vector<TrackingResult>& kNearest)
{
    kNearest.clear();
    scalar kDistance = 0;
    int index = 0;
    while(kNearest.size() < (unsigned int)k)
    {
        scalar mindist = FLT_MAX;
        for(unsigned int n = 0; n < dataSet.size(); n++)
        {
            scalar currDist = distance(A, dataSet[n]);
            if(mindist >= currDist && currDist > kDistance)
            {
                mindist = currDist;
                index = n;
            }
        }
        kNearest.push_back(dataSet[index]);
        kDistance = mindist;
    }
    return kDistance;
}

scalar kDistance(const std::vector<TrainingPair>& dataSet, const TrainingPair& A, int k, std::vector<TrainingPair>& kNearest)
{
    kNearest.clear();
    scalar kDistance = 0;
    int index = 0;
    while(kNearest.size() < (unsigned int)k)
    {
        scalar mindist = FLT_MAX;
        for(unsigned int n = 0; n < dataSet.size(); n++)
        {
            scalar currDist = distance(A.first, dataSet[n].first);
            if(mindist >= currDist && currDist > kDistance)
            {
                mindist = currDist;
                index = n;
            }
        }
        kNearest.push_back(dataSet[index]);
        kDistance = mindist;
    }
    return kDistance;
}

scalar rDistance(const std::vector<TrackingResult>& dataSet, const TrackingResult& A, const TrackingResult& B, int k)
{
    std::vector<TrackingResult> tmp;
    return std::max(kDistance(dataSet, B, k, tmp), distance(A,B));
}

scalar rDistance(const std::vector<TrainingPair>& dataSet, const TrainingPair& A, const TrainingPair& B, int k)
{
    std::vector<TrainingPair> tmp;
    return std::max(kDistance(dataSet, B, k, tmp), distance(A.first,B.first));
}

scalar lrd(const std::vector<TrackingResult>& dataSet, const TrackingResult& A, int k, std::vector<TrackingResult>& N_A)
{
    scalar result = 0.0;

    for(unsigned int n = 0; n < N_A.size(); n++)
        result += rDistance(dataSet, A, N_A[n], k);
    result /= N_A.size();

    return 1.0/result;
}


scalar lrd(const std::vector<TrainingPair>& dataSet, const TrainingPair& A, int k, std::vector<TrainingPair>& N_A)
{
    scalar result = 0.0;

    for(unsigned int n = 0; n < N_A.size(); n++)
        result += rDistance(dataSet, A, N_A[n], k);
    result /= N_A.size();

    return 1.0/result;
}

scalar lof(const std::vector<TrackingResult>& dataSet, const TrackingResult& A, int k)
{
    scalar result = 0.0;

    std::vector<TrackingResult> N_A;
    kDistance(dataSet, A, k, N_A);

    for(unsigned int n = 0; n < N_A.size(); n++)
        result += lrd(dataSet, N_A[n], k, N_A);
    result /= N_A.size();

    return result/lrd(dataSet, A, k, N_A);
}


scalar lof(const std::vector<TrainingPair>& dataSet, const TrainingPair& A, int k)
{
    scalar result = 0.0;

    std::vector<TrainingPair> N_A;
    kDistance(dataSet, A, k, N_A);

    for(unsigned int n = 0; n < N_A.size(); n++)
        result += lrd(dataSet, N_A[n], k, N_A);
    result /= N_A.size();

    return result/lrd(dataSet, A, k, N_A);
}


void parseFile (const std::string& mapFile,
                std::vector<TrainingPair>& knownPoints,
                std::vector<TrackingResult>& allPoints,
                std::vector<unsigned int>& mapping, int correction = 0)
{
    QFile mp(mapFile.c_str());    
    QString trackingFile(mapFile.c_str());
    trackingFile.chop(4);
    trackingFile += ".algo";
    QFile tr(trackingFile);

    QMap<int, QPointF> calibration;
    QMap<int, TrackingResult> gaze;

    if(!mp.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while(!mp.atEnd())
    {
        QString line(mp.readLine());
        line = line.trimmed();
        if(line.startsWith("#"))
            continue;
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt() - correction;
        QPointF point(splitted.at(1).toDouble(), splitted.at(2).toDouble());
        calibration.insert(frameNum, point);
    }

    mp.close();

    if(!tr.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    while(!tr.atEnd())
    {
        QString line(tr.readLine());
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt();
        TrackingResult res = TrackingResult::fromString(line.toStdString(), 1);
        if(res.glintConfidence() < 0.5 ||
                res.corner(TrackingResult::TopLeft).second == TrackingResult::Missing)
            continue;

        gaze.insert(frameNum, res);
    }

    tr.close();
    
    int lastFrame = 0;
    foreach(int f, calibration.keys())
    {
        if(!gaze.contains(f))
        {
            lastFrame = f;
            continue;
        }

        for(int n = lastFrame; n < f; n++)
            if(gaze.count(n) > 0)
                allPoints.push_back(gaze.value(n));

        mapping.push_back(allPoints.size());

        TrainingPair e;
        e.first = gaze.value(f);
        e.second = MappingResult(vector2(calibration.value(f).x(),
                                         calibration.value(f).y()), false);

        knownPoints.push_back(e);

        std::cout << "l;[" << lastFrame << "," << f << ")->"
                  << allPoints.size()-1 << ";" << knownPoints.size()-1
                  << std::endl;

        lastFrame = f;
    }
}

scalar rand_range(scalar min, scalar max)
{
    scalar val = rand()/(scalar)RAND_MAX;
    return min + (max-min)*val;
}

int main(int argc, char *argv[])
{
    std::vector<TrainingPair> knownPoints;
    std::vector<TrackingResult> allPoints;
    std::vector<unsigned int> mapping;

    for(int n = 1; n < argc - 1; n++)
    {
        QString str(argv[n]);
        parseFile(std::string(argv[n]), knownPoints, allPoints, mapping,  str.contains("calibration")? 0 : 1);
    }


    std::vector<TrainingPair> knownPointsTestCase;
    std::vector<TrackingResult> allPointsTestCase;
    std::vector<unsigned int> mappingTestCase;

    parseFile(std::string(argv[argc-1]), knownPointsTestCase, allPointsTestCase, mappingTestCase, 0);

    std::vector<TrainingPair> knownPointsFiltered;
    std::vector<TrackingResult> allPointsFiltered;

    for(unsigned int n = 0; n < knownPoints.size(); n++)
            knownPointsFiltered.push_back(knownPoints[n]);

    for(unsigned int n = 0; n < allPoints.size(); n++)
            allPointsFiltered.push_back(allPoints[n]);


        int maxNeuronCount = 8;

        timeval t1;
        gettimeofday(&t1, NULL);
        srand(t1.tv_usec * t1.tv_sec);

        scalar rolfR = 3.83497;
        scalar rolfLR = 0.00746023;
        scalar rolfLP = 0.0415123;
        scalar rolfIR = 3.81159;

        const unsigned int         neurons  = maxNeuronCount;
        const KernelType           kernel   = InvariantHeadMapper::Multiquadric;
        const PlacementStrategy    strategy = InvariantHeadMapper::ROLF;


        InvariantHeadMapper mapper(neurons, kernel, strategy,
                                   rolfR, rolfLR, rolfLP, rolfIR);

        mapper.addTrainingData(knownPointsFiltered);
        scalar diam = mapper.diameter();
        mapper.setKernelWidth(neurons/(diam*diam));

    std::cout << "# Training-vector count: " << knownPointsFiltered.size() << "\n";
    std::cout << "# Test-vector count: " << knownPointsTestCase.size() << "\n";


    typedef std::pair<MappingResult, MappingResult> temp;
    std::vector<temp> diffs;
    std::cout << "# Mapped values" << "\n";

    unsigned int old = 0;
    for(unsigned int n = 0; n < knownPointsTestCase.size(); n++)
    {
        TrainingPair e = knownPointsTestCase[n];
        MappingResult v = mapper.mapToScreen(e.first);
        diffs.push_back(std::make_pair(e.second, v));
        std::cout << "m;" << e.first.toString()
                  << ";" << v.toString() << "\n";
    }

    std::cout << "# Mapped differences" << "\n";
    scalar errX = 0.0;
    scalar errY = 0.0;
    foreach(temp t, diffs)
    {
        std::cout << "d;" << t.first.position().x << ";" << t.first.position().y
                  << ";" << t.second.position().x << ";" << t.second.position().y << "\n";
        errX += fabs(t.first.position().x - t.second.position().x);
        errY += fabs(t.first.position().y - t.second.position().y);
    }

    errX /= diffs.size();
    errY /= diffs.size();


    std::cout << "e;" << errX << ";" << errY << std::endl;

    return 0;
}
