#pragma once

#include <math.h>

static inline float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}


struct FractionalSincTable
{
protected:
    const int numFractions;
    float *coeffs;
    float normFreq;
public:
    const int N;
    FractionalSincTable(int N, int numFractions, float normFreq);
    virtual ~FractionalSincTable();
    const int getCoefficients(const float fraction, float* destinationBuffer, const int destinationBufferSize) const;
};
