#include "sinc.hpp"

#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

FractionalSincTable::FractionalSincTable(const int N, const int numFractions, float normFreq)
    : numFractions(numFractions)
    , coeffs(new float[numFractions*N])
    , normFreq(normFreq)
    , N(N)
{
    compute();
}

void FractionalSincTable::compute() {
    for (auto i = 0; i < numFractions; ++i)
    {
        float frac = (float)i*1.0f / (float(numFractions));
        float M = 0;
        for (auto n = 0; n < N; ++n)
        {
            int ni = i*N + n;
            float arg = ((float)n - frac - ((float)N - 1.f) / 2.f);
            float hamming = 0.54f + 0.46f*cosf((2.f*(float)M_PI*arg) / (float)(N - 1));
            coeffs[ni] = hamming*sincf(arg * normFreq);
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

    const float softIdx = fraction * (float)numFractions;
    assert(softIdx < numFractions);
    const int tableIdx1 = (int)softIdx;
    assert(tableIdx1 < numFractions);
    // TODO - linearly interpolate between neighbouring sets of coefficients

    for(int n=0; n<N; n++){
        int ni_1 = tableIdx1*N + n;
        destinationBuffer[n] = coeffs[ni_1];
    }

    return N;
}
