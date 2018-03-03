#ifndef RBFAPI_H
#define RBFAPI_H

#include "interfaces.h"
#include <set>
#include <vector>

//#define RBF_NORMALIZATION

class iBasisFunction
{
public:
    iBasisFunction(){}
    inline virtual
    scalar operator() (scalar value, scalar width) = 0;
};

template<int dimension>
class iMetric
{
public:
    iMetric(){}
    inline virtual
    scalar operator() (const vector<dimension>& a,
                       const vector<dimension>& b) = 0;
};

template<class B, template<int> class M, int dimension>
class RBFNeuron
{
public:
    RBFNeuron(const vector<dimension>& position, scalar width) :
        position(position), width(width) { }

    scalar operator() (const vector<dimension>& value)
    {
        return basisFunction(metric(position, value), width);
    }

private:
    vector<dimension> position;
    scalar width;
    B basisFunction;
    M<dimension> metric;
};

template<class B, template<int> class M, int dimension>
class RBFNet
{
public:
    RBFNet() {}

    void clear()
    {
        _neurons.clear();
    }

    void addNeuron(const vector<dimension>& position, scalar width, scalar weight)
    {
        _neurons.push_back(
                    std::make_pair(RBFNeuron<B,M,dimension>(position, width),
                                   weight));
    }

    scalar operator() (const vector<dimension>& input)
    {
        scalar result = 0.0;
#ifdef RBF_NORMALIZATION
        scalar normalize = 0.0;
#endif
        unsigned int count = _neurons.size();
        for(unsigned int n = 0; n < count; n++)
        {
            std::pair< RBFNeuron<B,M,dimension>, scalar> entry = _neurons[n];
            scalar val = entry.first(input);
            result += entry.second * val;
#ifdef RBF_NORMALIZATION
            normalize += val;
#endif
        }

#ifdef RBF_NORMALIZATION
            result /= normalize;
            result *= count;
#endif

        return result;
    }

private:
    std::vector< std::pair< RBFNeuron<B,M,dimension>, scalar> > _neurons;
};


template<class B, template<int> class M, int inputDimension, int outputDimension>
class RBFNetTrainer : public WeGA::Approximator<inputDimension, outputDimension>
{
public:
    RBFNetTrainer()
    {
        for(int n = 0; n < outputDimension; n++)
            _networks.push_back(0);
        clear();
    }

    virtual
    void clear()
    {
        _trainingData.clear();
        for(int n = 0; n < outputDimension; n++)
        {
            delete _networks[n];
            _networks[n] = new RBFNet<B,M,inputDimension>();
        }
    }

    void addTrainingTuple(const vector<inputDimension>&  in,
                          const vector<outputDimension>& out)
    {
        _trainingData.push_back(std::make_pair(in, out));
    }

    virtual
    void addInputValue(const vector<inputDimension>& in) = 0;

    virtual
    const std::vector<RBFNet<B,M,inputDimension>*>& networks()
    {
        return _networks;
    }

    virtual
    void update() = 0;

    vector<outputDimension> operator() (const vector<inputDimension>& input) const
    {
        vector<outputDimension> result;
        for(int n = 0; n < outputDimension; n++)
            result[n] = (*_networks[n])(input);
        return result;
    }

    vector<outputDimension> evaluate(const vector<inputDimension>& input) const
    {
        return (*this)(input);
    }

protected:
    std::vector<std::pair<vector<inputDimension>,vector<outputDimension> > > _trainingData;
    std::vector<RBFNet<B,M,inputDimension>*> _networks;
};

#endif // RBFAPI_H
