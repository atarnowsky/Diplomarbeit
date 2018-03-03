#include "cornersorter.h"

namespace WeGA {

scalar dist(vector2& p, vector2& r)
{
    scalar dx = p.x - r.x;
    scalar dy = p.y - r.y;
    scalar result = sqrt(dx*dx+dy*dy);
    return result;
}

CornerSorter::CornerSorter()
{
}


void CornerSorter::run(std::vector<vector2>& corners,
                       std::vector<TrackingResult::CornerStatus>& status)
{
    assert(corners.size() == 3);
    assert(status.size() == 3);

    vector2 min(FLT_MAX,FLT_MAX);
    vector2 max(-FLT_MAX,-FLT_MAX);

    std::vector<vector2> cCopy(corners);
    corners.push_back(vector2(0,0));
    status.push_back(TrackingResult::Missing);

    for(int n = 0; n < 3; n++)
        if(status[n] == TrackingResult::Found)
        {
            scalar x = cCopy[n].x;
            scalar y = cCopy[n].y;
            min.x = std::min(min.x, x);
            min.y = std::min(min.y, y);
            max.x = std::max(max.x, x);
            max.y = std::max(max.y, y);
        }

    vector2 points[] = {
        vector2(min),
        vector2(max.x, min.y),
        vector2(min.x, max.y),
        vector2(max)
    };

    for(int n = 0; n < 4; n++)
    {
        status[n] = TrackingResult::Missing;
    }

    for(int n = 0; n < 3; n++)
    {
        scalar minDist = dist(cCopy[n], points[0]);
        int minIndex = 0;
        for(int m = 1; m < 4; m++)
        {
            scalar curDist = dist(cCopy[n], points[m]);
            if(curDist < minDist)
            {
                minDist = curDist;
                minIndex = m;
            }
        }

        points[minIndex].x = INFINITY;
        corners[minIndex] = cCopy[n];
        status[minIndex] = TrackingResult::Found;
    }

    if(status[3] == TrackingResult::Missing)
    {
        vector2 mid((corners[0].x + corners[1].x) * 0.5,
                    (corners[0].y + corners[2].y) * 0.5);

        vector2 diff = corners[0] - mid;
        vector2 diff2 = corners[0] - corners[2];
        corners[3] = corners[1] - diff2;
        corners[3] += mid - diff;
        corners[3].x *= 0.5;
        corners[3].y *= 0.5;
        status[3] = TrackingResult::Interpolated;
    }

    if(status[0] == TrackingResult::Missing)
    {
        vector2 mid((corners[3].x + corners[2].x) * 0.5,
                    (corners[3].y + corners[1].y) * 0.5);

        vector2 diff = corners[3] - mid;
        vector2 diff2 = corners[3] - corners[1];
        corners[0] = corners[2] - diff2;
        corners[0] += mid - diff;
        corners[0].x *= 0.5;
        corners[0].y *= 0.5;
        status[0] = TrackingResult::Interpolated;
    }

    if(status[1] == TrackingResult::Missing)
    {
        vector2 mid((corners[3].x + corners[2].x) * 0.5,
                    (corners[0].y + corners[2].y) * 0.5);

        vector2 diff = corners[2] - mid;
        vector2 diff2 = corners[2] - corners[0];
        corners[1] = corners[3] - diff2;
        corners[1] += mid - diff;
        corners[1].x *= 0.5;
        corners[1].y *= 0.5;
        status[1] = TrackingResult::Interpolated;
    }

    if(status[2] == TrackingResult::Missing)
    {
        vector2 mid((corners[0].x + corners[1].x) * 0.5,
                    (corners[3].y + corners[1].y) * 0.5);

        vector2 diff = corners[1] - mid;
        vector2 diff2 = corners[1] - corners[3];
        corners[2] = corners[0] - diff2;
        corners[2] += mid - diff;
        corners[2].x *= 0.5;
        corners[2].y *= 0.5;
        status[2] = TrackingResult::Interpolated;
    }
}

}
