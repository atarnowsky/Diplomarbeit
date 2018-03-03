#ifndef DEFAULTMETRICS_H
#define DEFAULTMETRICS_H

#include "rbfapi.h"

template<int dimension>
class EuclideanMetric : public iMetric<dimension>
{
public:
    inline virtual
    scalar operator() (const vector<dimension>& a,
                       const vector<dimension>& b)
    {
        return cv::norm(a - b);
    }
};

template<int dimension>
class ManhattanMetric : public iMetric<dimension>
{
public:
    inline virtual
    scalar operator() (const vector<dimension>& a,
                       const vector<dimension>& b)
    {
        scalar result = 0.0;
        for(int n = 0; n < dimension; n++)
            result += fabs(a[n] - b[n]);
        return result;
    }
};

template<int dimension>
class ChebyshevMetric : public iMetric<dimension>
{
public:
    inline virtual
    scalar operator() (const vector<dimension>& a,
                       const vector<dimension>& b)
    {
        scalar result = 0.0;
        for(int n = 0; n < dimension; n++)
        {
            scalar val = fabs(a[n] - b[n]);
            if(val >= result)
                result = val;
        }
        return result;
    }
};

template<int dimension>
class DiscreteMetric : public iMetric<dimension>
{
public:
    inline virtual
    scalar operator() (const vector<dimension>& a,
                       const vector<dimension>& b)
    {
        for(int n = 0; n < dimension; n++)
        {
            if(fabs(a[n] - b[n]) >= SCALAR_EPS)
                return 1.0;
        }
        return 0.0;
    }
};

#endif // DEFAULTMETRICS_H
