#ifndef ROLFPLACER_H
#define ROLFPLACER_H

#include "approximationtrainer.h"

template<int inputDimension>
class ROLFPlacer : public NeuronPlacer<inputDimension>
{
protected:
    class ROLFNeuron
    {
    public:
        static bool ageSorter(const ROLFNeuron& d1, const ROLFNeuron& d2)
        {
          return d1.age < d2.age;
        }

        static bool sizeSorter(const ROLFNeuron& d1, const ROLFNeuron& d2)
        {
          return d1.radius < d2.radius;
        }

        ROLFNeuron(vector<inputDimension> center, scalar radius) :
            center(center), radius(radius), age(0) {}

        void adjust(vector<inputDimension> p,
                    scalar nyRho, scalar nyC)
        {
            scalar sum = 0;
            for(int q = 0; q < inputDimension; q++)
            {
                scalar diff = p[q] - center[q];
                sum += diff*diff;
            }

            center = center + nyC * (p - center);
            radius = radius + nyRho * (sqrt(sum) - radius);
            age = 0;
        }

        void pass(vector<inputDimension> p,
                  scalar nyRho, scalar nyC)
        {
            age++;
        }

        bool accepts(vector<inputDimension> p, scalar rho)
        {
            scalar sum = 0;
            for(int q = 0; q < inputDimension; q++)
            {
                scalar diff = p[q] - center[q];
                sum += diff*diff;
            }
            sum = sqrt(sum);

            return (sum <= radius * rho);
        }

        scalar distance(vector<inputDimension> p)
        {
            scalar sum = 0;
            for(int q = 0; q < inputDimension; q++)
            {
                scalar diff = p[q] - center[q];
                sum += diff*diff;
            }

            return sqrt(sum);
        }

        vector<inputDimension> center;
        scalar radius;
        unsigned int age;
    };

public:
    ROLFPlacer(scalar rho, scalar nyRho, scalar nyC, scalar initRadius, unsigned int neuronCount = 0) :
        _neuronCount(neuronCount), rho(rho), nyRho(nyRho), nyC(nyC), initRadius(initRadius)
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
        // Search for accepting neuron
        ROLFNeuron* acceptingNeuron = 0;
        scalar bestDistance = FLT_MAX;
        for(unsigned int n = 0; n < _rolfNeurons.size(); n++)
        {
            if(_rolfNeurons[n].accepts(in, rho))
            {
                scalar dist = _rolfNeurons[n].distance(in);
                if(dist <= bestDistance)
                {
                    bestDistance = dist;
                    acceptingNeuron = &_rolfNeurons[n];
                }
            }
        }

        // Adjust accepting neuron
        if(acceptingNeuron)
            acceptingNeuron->adjust(in, nyRho, nyC);
        // Create new one, if no one exists use initRadius. Otherwise use mean-r
        else
        {
            scalar meanR = 0;
            if(_rolfNeurons.size() > 0)
            {
                for(unsigned int n = 0; n < _rolfNeurons.size(); n++)
                    meanR += _rolfNeurons[n].radius;
                meanR /= _rolfNeurons.size();
            }
            else
                meanR = initRadius;

            _rolfNeurons.push_back(ROLFNeuron(in, meanR));
        }

        changed = true;
    }

    virtual
    void reset()
    {
        changed = true;
        _rolfNeurons.clear();
    }

    virtual
    const std::vector< vector<inputDimension> >& positions()
    {
        if(changed)
        {
            _positions.clear();
            _radii.clear();
            std::sort(_rolfNeurons.begin(), _rolfNeurons.end(), ROLFNeuron::sizeSorter);

            // Return only the most recently used neurons
            for(unsigned int n = 0; n < _neuronCount && n < _rolfNeurons.size(); n++)
            {
                _positions.push_back(_rolfNeurons[n].center);
                _radii.push_back(1.0/_rolfNeurons[n].radius*0.1);
            }
            changed = false;
        }

        return _positions;
    }

    virtual
    const std::vector< scalar >& radii(scalar baseSize)
    {
        _positions.clear();
        _radii.clear();
        std::sort(_rolfNeurons.begin(), _rolfNeurons.end(), ROLFNeuron::sizeSorter);

        // Return only the most recently used neurons
        for(unsigned int n = 0; n < _neuronCount && n < _rolfNeurons.size(); n++)
        {
            _positions.push_back(_rolfNeurons[n].center);
            _radii.push_back(initRadius/_rolfNeurons[n].radius*baseSize * 5);
        }
        changed = false;

        return _radii;
    }

    virtual
    scalar positionsDiameter()
    {
        positions();
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
    bool changed;
    unsigned int _neuronCount;
    scalar rho, nyRho, nyC;
    scalar initRadius;
    std::vector<ROLFNeuron> _rolfNeurons;
    std::vector< vector<inputDimension> > _positions;
    std::vector< scalar > _radii;
};

#endif // ROLFPLACER_H
