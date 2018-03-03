#ifndef CORNERSORTER_H
#define CORNERSORTER_H

#include "gazeapi.h"

class CornerSorter : public iCornerCompleter
{
public:
    CornerSorter();

protected:
    virtual
    void run(CornerSet& corners,
                        const cv::Point2f& pupilGlint);

    cv::Point2f lastCenter;
    CornerSet lastSet;
};

#endif // CORNERSORTER_H
