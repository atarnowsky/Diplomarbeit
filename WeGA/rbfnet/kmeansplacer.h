#ifndef KMEANSPLACER_H
#define KMEANSPLACER_H

#include "approximationtrainer.h"
#include <list>

template<int inputDimension>
class KMeansPlacer : public NeuronPlacer<inputDimension>
{
protected:
    inline scalar randf()
    {
        return (rand()/(scalar)(RAND_MAX));
    }

    struct clusterPair
    {
        vector<inputDimension>* point;
        std::vector< vector<inputDimension>* > trainingPoints;
    };

    bool equals(const std::vector<clusterPair>& a,
                const std::vector<clusterPair>& b)
    {
        if(a.size() != b.size())
            return false;

        for(unsigned int n = 0; n < a.size(); n++)
        {
            for(unsigned int m = 0; m < a[n].trainingPoints.size(); m++)
            {
                bool found = false;
                if(a[n].trainingPoints.size() != b[n].trainingPoints.size())
                    return false;
                for(unsigned int p = 0; p < b[n].trainingPoints.size(); p++)
                    if(a[n].trainingPoints[m] == b[n].trainingPoints[p])
                        found = true;
                if(!found)
                    return false;
            }
        }
        return true;
    }

    virtual
    void initialPlacement()
    {
        _positions.clear();
        if(!_useDataPositions || _training.size() < _neuronCount)
            for(unsigned int n = 0; n < _neuronCount; n++)
            {
                vector<inputDimension> val;
                for(int i = 0; i < inputDimension; i++)
                    val[i] = randf() * (_boundMax[i]-_boundMin[i]) + _boundMin[i];
                _positions.push_back(val);
            }
        else
        {
            std::vector< vector<inputDimension> > temp(_training.begin(), _training.end());
            std::vector<int> picks;
            for(unsigned int n = 0; n < _neuronCount; n++)
            {
                int num;
                bool found = true;
                while(found)
                {
                    num = rand() % _training.size();
                    std::vector<int>::iterator it = std::find( picks.begin(), picks.end(), num);
                    found = (it != picks.end());
                }
                picks.push_back(num);
                _positions.push_back(temp[num]);
            }
        }
    }

public:

    KMeansPlacer(bool useDataPositions, unsigned int neuronCount = 0, int bufferSize = -1) :
        _useDataPositions(useDataPositions), _neuronCount(neuronCount), _bufferSize(bufferSize)
    {
        reset();
    }

    virtual
    void setNeuronCount(unsigned int count)
    {
        _neuronCount = count;
    }

    virtual
    void addVector(const vector<inputDimension>& in)
    {
        for(int n = 0; n < inputDimension; n++)
        {
            _boundMin[n] = std::min(_boundMin[n], in[n]);
            _boundMax[n] = std::max(_boundMax[n], in[n]);
        }

        _training.push_back(in);
        if(_bufferSize >= 0 && _training.size() > (unsigned int)_bufferSize)
            _training.pop_front();

        changed = true;
    }

    virtual
    void reset()
    {
        for(int n = 0; n < inputDimension; n++)
        {
            _boundMin[n] = FLT_MAX;
            _boundMax[n] = -FLT_MAX;
        }
        _training.clear();
        changed = true;
    }

    virtual
    const std::vector< vector<inputDimension> >& positions()
    {
        if(!changed)
            return _positions;
        initialPlacement();

        std::vector<clusterPair> workingSet;
        std::vector<clusterPair> lastSet;
        for(unsigned int n = 0; n < _positions.size(); n++)
        {
            clusterPair temp;
            temp.point = &_positions[n];
            temp.trainingPoints.clear();
            workingSet.push_back(temp);
        }

        while(!equals(workingSet, lastSet))
        {
            lastSet = workingSet;

            for(unsigned int n = 0; n < workingSet.size(); n++)
            {
                workingSet[n].trainingPoints.clear();
            }

            typename std::list< vector<inputDimension> >::iterator it = _training.begin();
            for(; it != _training.end(); ++it)
            {
                scalar minDist = FLT_MAX;
                clusterPair* cluster = 0;
                for(unsigned int m = 0; m < workingSet.size(); m++)
                {
                    scalar sum = 0.0;
                    for(int p = 0; p < inputDimension; p++)
                    {
                        scalar diff = (*workingSet[m].point)[p] - (*it)[p];
                        sum += diff*diff;
                    }
                    scalar dist = sqrt(sum);
                    if(dist < minDist)
                    {
                        minDist = dist;
                        cluster = &workingSet[m];
                    }
                }

                cluster->trainingPoints.push_back(&*it);
            }

            for(unsigned int n = 0; n < workingSet.size(); n++)
            {
                if(workingSet[n].trainingPoints.size() == 0)
                    continue;
                vector<inputDimension> center;
                for(int p = 0; p < inputDimension; p++)
                    center[p] = 0.0;

                for(unsigned int m = 0; m < workingSet[n].trainingPoints.size(); m++)
                    center += *workingSet[n].trainingPoints[m];

                scalar scale = workingSet[n].trainingPoints.size();
                for(int p = 0; p < inputDimension; p++)
                    center[p] /= scale;

                *workingSet[n].point = center;
            }

        }
        changed = false;

        return _positions;
    }

    virtual
    const std::vector< scalar >& radii(scalar baseSize)
    {
        _radii.clear();
        for(unsigned int n = 0; n < _positions.size(); n++)
            _radii.push_back(baseSize);
        return _radii;
    }

    virtual
    scalar positionsDiameter()
    {
        scalar maximum = 0;
        for(unsigned int n = 0; n < _positions.size(); n++)
            for(unsigned int m = n; m < _positions.size(); m++)
            {
                vector<inputDimension> diff = _positions[n] - _positions[m];
                scalar dist = sqrt(diff.ddot(diff));
                if(dist > maximum)
                    maximum = dist;
            }

        return maximum;
    }

protected:
    bool changed, _useDataPositions;
    unsigned int _neuronCount;
    int _bufferSize;
    std::vector< vector<inputDimension> > _positions;
    std::vector< scalar > _radii;
    std::list< vector<inputDimension> > _training;
    vector<inputDimension> _boundMin;
    vector<inputDimension> _boundMax;
};

#endif // KMEANSPLACER_H
