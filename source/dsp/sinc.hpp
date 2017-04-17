#pragma once

#include <math.h>

static inline float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}

struct FractionalSincTable
{
private:
    const int N;
    const int numFractions;
    float *coeffs;

public:
    FractionalSincTable(int N, int numFractions);
    virtual ~FractionalSincTable();
    const int getCoefficients(const float fraction, float* destinationBuffer, const int destinationBufferSize) const;
};
