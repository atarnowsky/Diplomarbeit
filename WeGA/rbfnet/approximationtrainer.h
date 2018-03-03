#ifndef APPROXIMATIONTRAINER_H
#define APPROXIMATIONTRAINER_H

#include "rbfapi.h"
#include <eigen3/Eigen/Dense>

using namespace Eigen;

template<int inputDimension>
class /* interface */ NeuronPlacer
{
public:
    virtual
    void setNeuronCount(unsigned int count) = 0;
    virtual
    void addVector(const vector<inputDimension>& in) = 0;
    virtual
    void reset() = 0;

    virtual
    const std::vector< vector<inputDimension> >& positions() = 0;

    virtual
    const std::vector< scalar >& radii(scalar baseSize) = 0;

    virtual
    scalar positionsDiameter() = 0;
};

template<class B, template<int> class M, int inputDimension, int outputDimension>
class ApproximationTrainer : public RBFNetTrainer<B,M,inputDimension,outputDimension>
{
public:
    ApproximationTrainer(NeuronPlacer<inputDimension>& placer, scalar baseSize) :
        RBFNetTrainer<B,M,inputDimension,outputDimension>::RBFNetTrainer(),
        placer(placer), baseSize(baseSize)
    {

    }

    virtual
    void addInputValue(const vector<inputDimension>& in)
    {
        placer.addVector(in);
    }

    virtual
    void update() { update(baseSize); }

    virtual
    void update(scalar baseSize)
    {
        this->baseSize = baseSize;
        const std::vector< vector<inputDimension> >& placement =
                placer.positions();

        const std::vector< scalar >& radii =
                placer.radii(baseSize);

        if(placement.size() == 0)
            return;

        B fn; M<inputDimension> me;

        unsigned int trainingCount = this->_trainingData.size();
        unsigned int neuronCount   = placement.size();

        MatrixXf G(trainingCount, neuronCount);

        for(unsigned int j = 0; j < trainingCount; j++)
            for(unsigned int i = 0; i < neuronCount; i++)
            {
                double val = fn(me(
                             this->_trainingData[j].first,
                                    placement[i]
                                    ), radii[i]);

                G(j,i) = val;
            }

        for(int o = 0; o < outputDimension; o++)
        {
            VectorXf b(trainingCount);
            for(unsigned int bi = 0; bi < trainingCount; bi++)
                b(bi) = this->_trainingData[bi].second[o];

            VectorXf w = G.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);
            this->_networks[o]->clear();
            for(unsigned int n = 0; n < neuronCount; n++)
                this->_networks[o]->addNeuron(
                        placement[n],
                        radii[n],
                        w(n)
                        );
        }
    }


protected:
    NeuronPlacer<inputDimension>& placer;
    scalar baseSize;
};


#endif // APPROXIMATIONTRAINER_H
