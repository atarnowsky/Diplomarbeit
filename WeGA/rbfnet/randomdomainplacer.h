#ifndef RANDOMDOMAINPLACER_H
#define RANDOMDOMAINPLACER_H

#include "approximationtrainer.h"
#include <cstdlib>
#include <algorithm>

template<int inputDimension>
class RandomDomainPlacer : public NeuronPlacer<inputDimension>
{
private:
    inline scalar randf()
    {
        return (rand()/(scalar)(RAND_MAX));
    }

public:
    RandomDomainPlacer(bool useDataPositions, unsigned int neuronCount = 0) :
        _useDataPositions(useDataPositions),
        _neuronCount(neuronCount)
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
        if(changed)
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
                    _positions.push_back(_training[num]);
                }
            }
            changed = false;
        }

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
    std::vector< vector<inputDimension> > _positions;
    std::vector< scalar > _radii;
    std::vector< vector<inputDimension> > _training;
    vector<inputDimension> _boundMin;
    vector<inputDimension> _boundMax;
};

#endif // RANDOMDOMAINPLACER_H
