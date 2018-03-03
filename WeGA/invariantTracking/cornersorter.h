#ifndef CORNERSORTER_H
#define CORNERSORTER_H

#include "gazeapi.h"
#include "interfaces.h"

namespace WeGA {

class CornerSorter : public CornerCompleter
{
public:
    CornerSorter();

    virtual
    void run(std::vector<vector2>& corners,
             std::vector<TrackingResult::CornerStatus>& status);
};

}

#endif // CORNERSORTER_H
