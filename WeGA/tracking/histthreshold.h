#ifndef HISTTHRESHOLD_H
#define HISTTHRESHOLD_H

#include "interfaces.h"

namespace WeGA {

class HistThreshold : public ThresholdFilter
{
public:
    HistThreshold(scalar relArea, scalar scale = 1.0);
    ~HistThreshold();

    virtual
    scalar run(const cv::Mat& input, cv::Mat& output);

private:
    scalar relArea;
    scalar scale;
};

}

#endif // HISTTHRESHOLD_H
