#include "cornersorter.h"

inline float dist(cv::Point2f p, cv::Point2f  r)
{
    float dx = p.x - r.x;
    float dy = p.y - r.y;
    return sqrt(dx*dx+dy*dy);
}

CornerSorter::CornerSorter()
{
}


void CornerSorter::run(
    CornerSet& corners,
    const cv::Point2f& pupilGlint)
{
    int numCorners = 0;
    for(int n = 0; n < 4; n++)
        if(corners.status[n] >= 0)
            numCorners++;

    if(numCorners >= 3)
    {
        cv::Point2f min(9999,9999);
        cv::Point2f max(-9999,-9999);

        for(int n = 0; n < 4; n++)
            if(corners.status[n] >= 0)
            {
                float x = corners.point[n].x;
                float y = corners.point[n].y;
                min.x = std::min(min.x, x);
                min.y = std::min(min.y, y);
                max.x = std::max(max.x, x);
                max.y = std::max(max.y, y);
            }

        cv::Point2f points[] = {
            cv::Point2f(min),
            cv::Point2f(max.x, min.y),
            cv::Point2f(min.x, max.y),
            cv::Point2f(max)
        };

        lastCenter.x = (max.x + min.x)/2.0;
        lastCenter.y = (max.y + min.y)/2.0;

        CornerSet copy(corners);

        for(int n = 0; n < 4; n++)
        {
            corners.point[n].x = 0;
            corners.point[n].y = 0;
            corners.status[n] = -1;
        }

        for(int n = 0; n < 4; n++)
            if(copy.status[n] >= 0)
            {
                float minDist = dist(copy.point[n], points[0]);
                int minIndex = 0;
                for(int m = 1; m < 4; m++)
                {
                    float curDist = dist(copy.point[n], points[m]);
                    if(curDist < minDist)
                    {
                        minDist = curDist;
                        minIndex = m;
                    }
                }

                points[minIndex].x = INFINITY;
                corners.point[minIndex] = copy.point[n];
                corners.status[minIndex] = copy.status[n];
            }
    }
    else
    {
        cv::Point2f displacement(0,0);
        int norm = 0;
        for(int n = 0; n < 4; n++)
            if(corners.status[n] == 0 && lastSet.status[n] >= 0)
            {
                displacement += corners.point[n] - lastSet.point[n];
                norm++;
            }

        displacement.x /= norm;
        displacement.y /= norm;

        lastCenter += displacement;

        CornerSet copy(corners);

        for(int n = 0; n < 4; n++)
        {
            corners.point[n].x = 0;
            corners.point[n].y = 0;
            corners.status[n] = -1;
        }

        for(int n = 0; n < 4; n++)
            if(copy.status[n] >= 0)
            {
                float x = copy.point[n].x;
                float y = copy.point[n].y;
                int index = n;
                if(x < lastCenter.x && y < lastCenter.y) index = 0;
                if(x > lastCenter.x && y < lastCenter.y) index = 1;
                if(x < lastCenter.x && y > lastCenter.y) index = 2;
                if(x > lastCenter.x && y > lastCenter.y) index = 3;

                corners.point[index] = copy.point[n];
                corners.status[index] = copy.status[n];
            }

    }

    for(int n = 0; n < 4; n++)
    {
        lastSet.point[n] = corners.point[n];
        lastSet.status[n] = corners.status[n];
    }
}
