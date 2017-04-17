#include "sinc.hpp"

#include <assert.h>

FractionalSincTable::FractionalSincTable(const int N, const int numFractions)
    : N(N)
    , numFractions(numFractions)
    , coeffs(new float[numFractions*N])
{
    for (auto i = 0; i < numFractions; ++i)
    {
        float frac = (float)i*1.0f/(float(numFractions));
        float M = 0;
        for (auto n = 0; n < N; ++n)
        {
            int ni = i*N + n;
            float arg = (float)n-frac-((float)N-1.f)/2.f;
            float hamming = 0.54f+0.46f*cosf( (2.f*M_PI*arg) / (float)(N-1) );
            coeffs[ni] = hamming*sincf(arg);
            M += coeffs[ni];
        }
        // silly normalization
        for (auto n = 0; n < N; ++n)
        {
            int ni = i*N + n;
            coeffs[ni] /= M;
        }
    }
}

FractionalSincTable::~FractionalSincTable(){
    delete[] coeffs;
}

const int FractionalSincTable::getCoefficients(const float fraction, float* destinationBuffer, const int destinationBufferSize) const {
    if(destinationBufferSize < N)
    {
        return 0;
    }
    assert(fraction < 1.0f && fraction >= 0.0f);

    // linearly interpolate between neighbouring sets of coefficients
    const float softIdx = fraction * (float)numFractions;
    assert(softIdx < numFractions);
    const int tableIdx1 = (int)softIdx;
    const int tableIdx2 = ((tableIdx1+1) < numFractions) ? (tableIdx1+1) : 0;
    assert(tableIdx1 < numFractions);
    assert(tableIdx2 < numFractions);

    const float ratio = softIdx - tableIdx1;
    assert(ratio < 1.0f && ratio >= 0.0f);

    for(int n=0; n<N; n++){
        int ni_1 = tableIdx1*N + n;
        int ni_2 = tableIdx2*N + n;
        destinationBuffer[n] = (1 - ratio) * coeffs[ni_1] + ratio*coeffs[ni_2];
    }

    return N;
}
