#include <iostream>
#include <vector>
#include <QFile>
#include <QString>
#include <QMultiHash>
#include <QPoint>
#include <QStringList>
#include <cstdlib>

#include "WeGA/wega.h"

using namespace WeGA;
using WeGA::FixedHeadMapper;

#define REMOVE_OUTLIERS

uint qHash(const QPointF& p)
{
    return (uint)qRound(p.x()*p.y()*10000);
}

std::vector<FixedHeadMapper::TrainingPair> loadCalibration(const std::string& mapFile,
                                              const std::string& calibrationFile)
{
    std::vector<FixedHeadMapper::TrainingPair> result;
    QFile mp(mapFile.c_str());
    QFile cl(calibrationFile.c_str());

    QMultiHash<QPointF, int> calibration;
    QHash<int, TrackingResult> gaze;

    if(!mp.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    while(!mp.atEnd())
    {
        QString line(mp.readLine());
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt() + 1;
        QPointF point(splitted.at(1).toDouble(), splitted.at(2).toDouble());
        calibration.insert(point, frameNum);
    }

    mp.close();

    if(!cl.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    while(!cl.atEnd())
    {
        QString line(cl.readLine());
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt();
        TrackingResult res = TrackingResult::fromString(line.toStdString(), 1);
        gaze.insert(frameNum, res);
    }

    cl.close();

    QPointF lastPoint(-999999,-999999);

    std::cout << "# Calibration points:" << std::endl;

    foreach(QPointF p, calibration.keys())
    {
        if(p == lastPoint) continue;
        QPointF gazePos;

        foreach(int frame, calibration.values(p))
        {
            gazePos += QPointF(gaze.value(frame).pupilGlintVector().x,
                               gaze.value(frame).pupilGlintVector().y);
        }
        gazePos /= calibration.count(p);

        QPointF stdDev(0.0, 0.0);
        foreach(int frame, calibration.values(p))
        {
            QPointF valDiff(QPointF(gaze.value(frame).pupilGlintVector().x,
                                    gaze.value(frame).pupilGlintVector().y) - gazePos);
            stdDev.setX(stdDev.x() + valDiff.x()*valDiff.x());
            stdDev.setY(stdDev.y() + valDiff.y()*valDiff.y());
        }
        stdDev.setX(sqrt(stdDev.x()/(calibration.count(p)-1)));
        stdDev.setY(sqrt(stdDev.y()/(calibration.count(p)-1)));

        QPointF gazePosOld = gazePos;
        gazePos = QPointF(0,0);
        int counter = 0;
        foreach(int frame, calibration.values(p))
        {
            if((fabs(gaze.value(frame).pupilGlintVector().x - gazePosOld.x()) <= stdDev.x())
                    && (fabs(gaze.value(frame).pupilGlintVector().y - gazePosOld.y()) <= stdDev.y()))
            {
                gazePos += QPointF(gaze.value(frame).pupilGlintVector().x,
                                   gaze.value(frame).pupilGlintVector().y);
                counter++;
            }
        }
        gazePos /= counter;
        std::cout << "# Removed " << calibration.count(p) - counter << " calibration points." << std::endl;

        vector2 corners[4] = {vector2(0,0), vector2(0,0), vector2(0,0), vector2(0,0)};
        TrackingResult::CornerStatus status[4] = {
            TrackingResult::Missing, TrackingResult::Missing,
            TrackingResult::Missing, TrackingResult::Missing
        };

        FixedHeadMapper::TrainingPair e;
        e.first = TrackingResult(vector2(gazePos.x(), gazePos.y()), vector2(0,0),
                                 corners, status, 1.0);
        e.second = MappingResult(vector2(p.x(), p.y()), false);
        result.push_back(e);

        std::cout << "c;" << e.second.position().x << ";" << e.second.position().y
                  << ";" << e.first.pupilGlintVector().x << ";" << e.first.pupilGlintVector().y
                  << ";" << stdDev.x() << ";" << stdDev.y() << std::endl;

        lastPoint = p;
    }

    return result;
}

typedef std::pair<int, FixedHeadMapper::TrainingPair> lstPair;

bool teLessThan(const lstPair &s1, const lstPair &s2)
 {
     return s1.first < s2.first;
 }

std::vector<FixedHeadMapper::TrainingPair> loadTestpattern(const std::string& mapFile,
                                              const std::string& patternFile)
{
    std::vector<FixedHeadMapper::TrainingPair> result;
    std::vector<lstPair> resultTemp;
    QFile mp(mapFile.c_str());
    QFile pt(patternFile.c_str());

    QMultiHash<QPointF, int> pattern;
    QHash<int, TrackingResult> gaze;

    if(!mp.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    while(!mp.atEnd())
    {
        QString line(mp.readLine());
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt();
        QPointF point(splitted.at(1).toDouble(), splitted.at(2).toDouble());
        pattern.insert(point, frameNum);
    }

    mp.close();

    if(!pt.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    while(!pt.atEnd())
    {
        QString line(pt.readLine());
        QStringList splitted(line.split(';'));
        int frameNum = splitted.at(0).toInt();
        TrackingResult res = TrackingResult::fromString(line.toStdString(), 1);
        gaze.insert(frameNum, res);
    }

    pt.close();

    QPointF lastPoint(-999999,-999999);

    std::cout << "# Testpattern points:" << std::endl;

    foreach(QPointF p, pattern.keys())
    {
        if(p == lastPoint) continue;

        foreach(int frame, pattern.values(p))
        {
            TrackingResult r = gaze.value(frame);
            lstPair e;
            e.first = frame;
            e.second.first = r;
            e.second.second = MappingResult(vector2(p.x(), p.y()), false);
            resultTemp.push_back(e);
        }
        lastPoint = p;
    }

    qStableSort(resultTemp.begin(), resultTemp.end(), teLessThan);

    foreach(lstPair e, resultTemp)
    {
        std::cout << "t;" << e.second.second.position().x
                  << ";"  << e.second.second.position().y
                  << ";"  << e.second.first.pupilGlintVector().x
                  << ";"  << e.second.first.pupilGlintVector().y
                  << ";"  << e.first
                  << std::endl;
        result.push_back(e.second);
    }

    return result;
}

scalar rateResult(std::vector<FixedHeadMapper::TrainingPair> ref, std::vector<MappingResult> res)
{
    QMultiHash<QPointF, QPointF> results;

    for(int n = 0; n < ref.size(); n++)
    {
        results.insert(QPointF(ref[n].second.position().x, ref[n].second.position().y),
                       QPointF(res[n].position().x, res[n].position().y));
    }

    QHash<QPointF, QPointF> meanValues;
    QHash<QPointF, QPointF> stdDeviations;

    foreach(QPointF p, results.keys())
    {
        QList<QPointF> all(results.values(p));
        QPointF mean(0,0);
        foreach(QPointF p2, all)
            mean += p2;
        mean /= all.size();
        meanValues.insert(p, mean);
    }

    foreach(QPointF p, results.keys())
    {
        QList<QPointF> all(results.values(p));
        QPointF mean = meanValues.value(p);
        QPointF stdDev(0,0);
        foreach(QPointF p2, all)
            stdDev += QPointF((p2.x() - mean.x()) * (p2.x() - mean.x()),
                              (p2.y() - mean.y()) * (p2.y() - mean.y()));
        stdDev /= all.size() - 1;
        stdDeviations.insert(p, QPointF(sqrt(stdDev.x()), sqrt(stdDev.y())));
    }

    QMultiHash<QPointF, QPointF> filteredValues;
    QHash<QPointF, QPointF> finalPoints;

    foreach(QPointF p, results.keys())
    {
        QList<QPointF> all(results.values(p));
        QPointF mean = meanValues.value(p);
        QPointF stdDev = stdDeviations.value(p);
        foreach(QPointF p2, all)
        {
            QPointF diff = p2-mean;
            if((fabs(diff.x()) < stdDev.x()*2) &&
                    (fabs(diff.y()) < stdDev.y()*2))
                filteredValues.insert(p, p2);
        }
    }

    foreach(QPointF p, filteredValues.keys())
    {
        QList<QPointF> all(filteredValues.values(p));
        QPointF mean(0,0);
        foreach(QPointF p2, all)
            mean += p2;
        mean /= all.size();
        finalPoints.insert(p, mean);
    }

    QPointF displacement(0,0);
    foreach(QPointF p, finalPoints.keys())
    {
        displacement += (p - finalPoints.value(p));
    }
    displacement /= finalPoints.count();

    foreach(QPointF p, finalPoints.keys())
    {
        QPointF corrected = finalPoints.value(p) + displacement;
        finalPoints.insert(p, corrected);
    }


    scalar totalErr = 0.0;
    foreach(QPointF p, finalPoints.keys())
    {
        QPointF t = (finalPoints.value(p) - p);
        totalErr += sqrt(t.x()*t.x()+t.y()*t.y());
    }
    totalErr /= finalPoints.count();
//std::cerr << totalErr << std::endl;
    return totalErr;
}


int main(int argc, char *argv[])
{
    std::string calibrationmap(argv[1]);
    std::string calibrationfile(argv[2]);
    std::string trackingmap(argv[3]);
    std::string trackingfile(argv[4]);
    unsigned char method = argv[5][0];

    FixedHeadMapper::KernelType type = FixedHeadMapper::Linear;

    switch(method)
    {
    case 'L':
        type = FixedHeadMapper::Linear;
        break;
    case 'G':
        type = FixedHeadMapper::Gaussian;
        break;
    case 'M':
        type = FixedHeadMapper::Multiquadric;
        break;
    case 'S':
        type = FixedHeadMapper::Spheric;
        break;
    case 'A':
        type = FixedHeadMapper::AutoSpheric;
        break;
    case 'C':
        type = FixedHeadMapper::LinearCGAL;
        break;
    }

    std::vector<FixedHeadMapper::TrainingPair> calibrationList =
            loadCalibration(calibrationmap, calibrationfile);

    std::vector<FixedHeadMapper::TrainingPair> patternList =
            loadTestpattern(trackingmap, trackingfile);


    FixedHeadMapper mapper(type);    
    mapper.addTrainingData(calibrationList);

    scalar widthFactor = 1.0/mapper.trainingDataDiameter();

    if(type == FixedHeadMapper::AutoSpheric)
    {
        mapper.sphericOptimization();
    }
    else if(type != FixedHeadMapper::LinearCGAL)
    {
        scalar bestWidth = 1.0;
        scalar bestError = FLT_MAX;

        std::cout << "# Width induced errors" << std::endl;
        for(scalar w = 0.015; w < 1.5; w += 0.001)
        {
            mapper.setKernelWidth(w*widthFactor);
            scalar totalError = 0.0;
            std::vector<MappingResult> results;
            foreach(FixedHeadMapper::TrainingPair e, patternList)
            {
                results.push_back(mapper.mapToScreen(e.first));
                //MappingResult v = mapper.mapToScreen(e.first);
                /*totalError += fabs(e.second.position().x - v.position().x) +
                              fabs(e.second.position().y - v.position().y);*/
            }

            totalError = rateResult(patternList, results);
            if(totalError < bestError)
            {
                bestError = totalError;
                bestWidth = w;
            }

            std::cout << "w;" << w*widthFactor
                      << ";" << totalError << std::endl;
        }

        std::cout << "# Best kernel-width:" << bestWidth << ", factor: " << widthFactor << "\n";

        mapper.setKernelWidth(bestWidth * widthFactor);
    }


    typedef std::pair<MappingResult, MappingResult> temp;
    std::vector<temp> diffs;
    std::cout << "# Mapped values\n";
    foreach(FixedHeadMapper::TrainingPair e, patternList)
    {
        MappingResult v = mapper.mapToScreen(e.first);
        diffs.push_back(std::make_pair(e.second, v));
        std::cout << "m;" << e.first.pupilGlintVector().x
                  << ";" << e.first.pupilGlintVector().y
                  << ";" << v.position().x << ";" << v.position().y << "\n";
    }

    std::cout << "# Mapped differences" << "\n";
    foreach(temp t, diffs)
    {
        std::cout << "d;" << t.first.position().x << ";" << t.first.position().y
                  << ";" << t.second.position().x << ";" << t.second.position().y << "\n";
    }

    std::cout << "# data diameter " << mapper.trainingDataDiameter() << std::endl;


    return 0;
}
