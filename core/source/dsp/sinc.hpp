#pragma once

#include <math.h>
#include <assert.h>

static inline float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}

template <int N, int numFractions>
struct FractionalSincTable
{
protected:
    float *coeffs;
    const float normFreq;
    const bool normAmp;

public:

    FractionalSincTable(float normFreq, bool normAmp = true) 
        : coeffs(new float[numFractions*N])
        , normFreq(normFreq)
        , normAmp(normAmp)
    {
        for (auto i = 0; i < numFractions; ++i)
        {
            float frac = (float)i*1.0f / (float(numFractions));
            float M = 0.f;
            for (auto n = 0; n < N; ++n)
            {
                int ni = i*N + n;
                float arg = ((float)n - frac - ((float)N - 1.f) / 2.f);
                float hamming = 0.54f + 0.46f*cosf((2.f*(float)M_PI*arg) / (float)(N - 1));
                coeffs[ni] = hamming*sincf(arg * normFreq);
                M += coeffs[ni];
            }
            if (normAmp) {
                for (auto n = 0; n < N; ++n)
                {
                    int ni = i*N + n;
                    coeffs[ni] /= M;
                }
            }
        }
    };

    virtual ~FractionalSincTable() {
        delete[] coeffs;
    };

    const int getCoefficients(const float fraction, float* destinationBuffer, const int destinationBufferSize) const {
        if (destinationBufferSize < N)
        {
            return 0;
        }
        if (fraction > 1.0f || fraction < 0.f) {
            assert(0);
            return 0;
        }

        const float softIdx = fraction * (float)numFractions;
        assert(softIdx < numFractions);
        const int tableIdx1 = (int)softIdx;
        assert(tableIdx1 < numFractions);
        // TODO - linearly interpolate between neighbouring sets of coefficients

        for (int n = 0; n<N; n++) {
            int ni_1 = tableIdx1*N + n;
            destinationBuffer[n] = coeffs[ni_1];
        }

        return N;
    };

};
