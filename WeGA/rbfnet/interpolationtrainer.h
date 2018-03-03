#ifndef INTERPOLATIONTRAINER_H
#define INTERPOLATIONTRAINER_H

#include "rbfapi.h"

template<class B, template<int> class M, int inputDimension, int outputDimension>
class InterpolationTrainer : public RBFNetTrainer<B,M,inputDimension,outputDimension>
{
public:
    InterpolationTrainer(scalar baseSize) : RBFNetTrainer<B,M,inputDimension,outputDimension>::RBFNetTrainer()
    {
        lastTrainingCount = 0;
        this->baseSize = baseSize;
    }

    virtual
    void addInputValue(const vector<inputDimension>& in) {}

    virtual
    void update()
    {
        update(baseSize);
    }

    virtual
    void update(scalar baseSize)
    {        
        if(this->_trainingData.size() == 0)
        {
            this->baseSize = baseSize;
            return;
        }

        if(this->_trainingData.size() != lastTrainingCount || baseSize != this->baseSize)
        {
            this->baseSize = baseSize;
            B fn; M<inputDimension> me;
            //TODO: Seperate this loop. G.inv() only has to be calculated once!
            for(int n = 0; n < outputDimension; n++)
            {   // Train each network separately
                unsigned int size = this->_trainingData.size();
                //TODO: Check if all trainingData elements are distinct
                cv::Mat b(size,1,CV_64F);
                cv::Mat G(size,size,CV_64F);

                for(unsigned int m = 0; m < size; m++)
                {
                    b.at<double>(m,0) = this->_trainingData[m].second[n];
                }

                for(unsigned int i = 0; i < size; i++)
                    for(unsigned int j = i; j < size; j++)
                    {
                        double val = fn(me(
                                     this->_trainingData[i].first,
                                     this->_trainingData[j].first
                                            ), baseSize);

                        G.at<double>(i,j) = val;
                        G.at<double>(j,i) = val;
                    }

                cv::Mat w;
                w = G.inv(cv::DECOMP_LU)*b;

                this->_networks[n]->clear();
                for(unsigned int m = 0; m < size; m++)
                    this->_networks[n]->addNeuron(
                            this->_trainingData[m].first,
                            baseSize,
                            w.at<double>(m,0)
                            );
            }
        }

        lastTrainingCount = this->_trainingData.size();
    }

protected:
    unsigned int lastTrainingCount;
    scalar baseSize;
};

#endif // INTERPOLATIONTRAINER_H
