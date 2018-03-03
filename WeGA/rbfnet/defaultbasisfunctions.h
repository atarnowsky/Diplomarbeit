#ifndef DEFAULTBASISFUNCTIONS_H
#define DEFAULTBASISFUNCTIONS_H

#include "rbfapi.h"
#include <cmath>

class GaussianFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return exp(-(width*value)*(width*value));
    }
};


class LinearFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        if(value <= 0.00001)
            return 0.0;
        return value*value*log(value);
    }
};

class MultiquadricFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return sqrt(scalar(1.0) + (width*value)*(width*value));
    }
};

class BumpFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {        
        if(width*value*width*value >= 1.0)
            return 0.0;
        return exp(-1.0/(1.0-(width*value)*(width*value)));
    }
};


class QuadraticFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return pow(value, 2.0);
    }
};

class InverseMultiquadricFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return scalar(1.0) / sqrt(scalar(1.0) + (width*value)*(width*value));
    }
};

class InverseQuadraticFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return scalar(1.0) / (scalar(1.0) + (width*value)*(width*value));
    }
};

class ThinPlateSplineFunction : public iBasisFunction
{
public:
    inline virtual
    scalar operator() (scalar value, scalar width)
    {
        return value*value*log(value);
    }
};

#endif // DEFAULTBASISFUNCTIONS_H
