#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include <assert.h>

static inline float sincf(float x){
    return x!=0 ? sinf(x)/x : 1;
}

const int c_numFractions = 1000;

static inline int makeIndex(int N, int n, int i) {
    return i*N + n;
}

template <int N>
struct FractionalSincTable
{
protected:
    float *coeffs;
    const float normFreq;
    const bool normAmp;

public:

    FractionalSincTable(float normFreq = (float)M_PI, bool normAmp = true)
        : coeffs(new float[c_numFractions*N])
        , normFreq(normFreq)
        , normAmp(normAmp)
    {
        for (auto i = 0; i < c_numFractions; ++i)
        {
            float frac = (float)i*1.0f / (float(c_numFractions));
            float M = 0.f;
            for (auto n = 0; n < N; ++n)
            {
                int ni = makeIndex(N, n, i);
                float arg = ((float)n - frac - ((float)N - 1.f) / 2.f);
                float hamming = 0.54f + 0.46f*cosf((2.f*(float)M_PI*arg) / (float)(N - 1));
                coeffs[ni] = hamming*sincf(arg * normFreq);
                M += coeffs[ni];
            }
            if (normAmp) {
                for (auto n = 0; n < N; ++n)
                {
                    int ni = makeIndex(N, n, i);
                    coeffs[ni] /= M;
                }
            }
        }
    };

    virtual ~FractionalSincTable() {
        delete[] coeffs;
    };

    int getCoefficientTablePointer(const float fraction, float** destinationBuffer, const int destinationBufferSize) const {
        if (destinationBufferSize < N)
        {
            assert(0);
            return 0;
        }
        if (fraction > 1.0f || fraction < 0.f) {
            assert(0);
            return 0;
        }

        const float softIdx = fraction * (float)c_numFractions;
        assert(softIdx < c_numFractions);
        const int i = (int)softIdx;
        assert(i < c_numFractions);
        
        int ni_0 = makeIndex(N, 0, i);
        *destinationBuffer = &coeffs[ni_0];

        return N;
    }

};

template <int N>
const FractionalSincTable<N> & getFractionalSincTable();
